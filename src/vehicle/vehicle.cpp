/**
 * @file vehicle.cpp
 * @brief Implementation of Vehicle class
 * @author Traffic Simulation Team
 * @version 1.0
 */

#include "vehicle/vehicle.hpp"
#include "vehicle/pathfinder.hpp"
#include <cmath>
#include <iostream>
#include "common/logger.hpp"
#include "city/street.hpp"

namespace vehicle {

Vehicle::Vehicle(int id)
    : id_(id),
      position_(city::Coordinate{0, 0}),
      destination_(city::Coordinate{0, 0}),
      state_(traffic_simulation::VehicleState::WAITING),
      path_index_(0),
      city_(nullptr),
      total_travel_time_(0.0),
      wait_time_(0.0),
      is_waiting_(false),
      running_(false),
      conditions_changed_(false) {
}


Vehicle::~Vehicle() {
    stopThread();
}


bool Vehicle::setRoute(const city::Coordinate& start,
                       const city::Coordinate& dest,
                       city::City& city) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    position_ = start;
    destination_ = dest;
    path_index_ = 0;
    city_ = &city;
    
    // Use A* to find path
    current_path_ = Pathfinder::findPath(start, dest, city);
    
    if (current_path_.empty()) {
        state_ = traffic_simulation::VehicleState::BLOCKED;
        return false;
    }

    // Claim the starting cell immediately so no other vehicle can spawn here.
    auto start_intersection = city_->getIntersection(start);
    if (start_intersection && !start_intersection->try_lock()) {
        // Cell already occupied; caller must retry with a different start.
        current_path_.clear();
        state_ = traffic_simulation::VehicleState::BLOCKED;
        return false;
    }

    state_ = traffic_simulation::VehicleState::WAITING;
    return true;
}


void Vehicle::setPausedFlag(std::atomic<bool>* flag) {
    global_paused_ = flag;
}



void Vehicle::run() {
    start_time_ = std::chrono::steady_clock::now();
    
    while (running_) {
        auto current_state = state_.load();
        
        if (current_state == traffic_simulation::VehicleState::ARRIVED) {
            break;
        }
        
        if (current_state == traffic_simulation::VehicleState::WAITING ||
            current_state == traffic_simulation::VehicleState::BLOCKED) {
            
            // Wait until a tick is requested OR running becomes false
            {
                std::unique_lock<std::mutex> lk(state_mutex_);
                cv_.wait(lk, [this]{
                    return tick_requested_.load() || !running_.load();
                });
                tick_requested_ = false;
            }
            
            if (!running_.load()) break;
            
            // Try to advance once
            if (city_ && canAdvance(*city_, semaphore_controller_)) {
                if (advance()) {
                    blocked_ticks_ = 0;
                    continue;
                }
            }
            
            // Handle blocking (same as before: light check, deadlock recovery, wait time)
            bool blocked_by_light = false;
            if (city_ && semaphore_controller_ && path_index_ < current_path_.size()) {
                auto next_pos = current_path_[path_index_];
                auto semaphore = semaphore_controller_->getSemaphoreAt(next_pos);
                if (semaphore && !semaphore->isGreen()) {
                    blocked_by_light = true;
                }
            }

            if (!blocked_by_light) {
                blocked_ticks_++;
                
                if (blocked_ticks_ > 15 && path_index_ < current_path_.size()) {
                    auto node_to_avoid = current_path_[path_index_];
                    traffic_simulation::Logger::getInstance().warning(
                        "Vehículo " + std::to_string(id_) + " deadlock recovery. Re-calculando ruta...");
                    auto new_path = Pathfinder::findPath(position_, destination_, *city_, node_to_avoid);
                    if (!new_path.empty() && new_path.size() > 1) {
                        std::lock_guard<std::mutex> lock(state_mutex_);
                        current_path_ = new_path;
                        path_index_ = 1;
                        blocked_ticks_ = 0;
                    }
                }
            }
            
            if (!is_waiting_.load()) {
                wait_start_ = std::chrono::steady_clock::now();
                is_waiting_ = true;
            }
            // Event-driven wait: sleep until light turns green, then re-evaluate.
            // Falls back to a short sleep only when blocked by intersection occupancy
            // (the 500ms try_lock_for in advance() already handles that timeout).
            if (semaphore_controller_ && path_index_ < current_path_.size()) {
                auto next_pos = current_path_[path_index_];
                auto sem = semaphore_controller_->getSemaphoreAt(next_pos);
                if (sem && !sem->isGreen()) {
                    sem->waitForGreen();  // shared_lock + condition_variable_any
                    continue;             // re-enter loop and retry canAdvance
                }
            }
            // Intersection is occupied by another vehicle (light is green but lock failed).
            // Short sleep before retry; try_lock_for already backed off 500ms in advance().
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        else if (current_state == traffic_simulation::VehicleState::MOVING) {
            // After a tick-advancement, immediately set to WAITING
            // (No more self-sleep, the tick loop controls timing)
            setState(traffic_simulation::VehicleState::WAITING);
        }
        
        // Check if arrived (same as before)
        if (isArrived()) {
            arrival_time_ = std::chrono::steady_clock::now();
            auto travel_duration = arrival_time_ - start_time_;
            // Usamos .store() por ser atomic en C++17
            total_travel_time_.store(std::chrono::duration<double>(travel_duration).count());
            
            // NUEVO: Libera bloqueo de la intersección destino final
            // Cerrar el cronómetro de espera si llegó al destino mientras estaba esperando
            if (is_waiting_.load()) {
                auto wait_end = std::chrono::steady_clock::now();
                double added_time = std::chrono::duration<double>(wait_end - wait_start_).count();
                wait_time_.store(wait_time_.load() + added_time);
                is_waiting_ = false;
            }

            // --- ENVIAR MÉTRICAS ---
            if (metrics_collector_ != nullptr) {
                metrics_collector_->recordVehicleArrival(id_, total_travel_time_.load(), wait_time_.load());
            } else {
                // Si vemos esto en consola, el puntero nunca llegó
                std::cout << "\n[ERROR INTERNO] metrics_collector_ es NULL en Vehículo " << id_ << "\n";
            }

            if (city_) {
                auto final_intersection = city_->getIntersection(position_);
                if (final_intersection) {
                    final_intersection->unlock();
                }
            }

            setState(traffic_simulation::VehicleState::ARRIVED);
            break;
        }
    }
}

void Vehicle::tick() {
    tick_requested_ = true;
    cv_.notify_one();
}

bool Vehicle::canAdvance(city::City& city, const traffic::SemaphoreController* semaphore_controller) {
    if (current_path_.empty() || path_index_ >= current_path_.size()) {
        return false;
    }
    
    // Get next position in path
    auto next_pos = current_path_[path_index_];
    
    // Check if next position is adjacent (one step)
    int dx = std::abs(next_pos.x - position_.x);
    int dy = std::abs(next_pos.y - position_.y);
    
    if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1)) {
        // Next position is adjacent - need to check intersection availability
        auto intersection = city.getIntersection(next_pos);
        if (intersection) {
            // Check if intersection is available
            if (!intersection->isAvailable()) {
                return false;
            }
            // Check traffic light if semaphore controller is provided
            if (semaphore_controller) {
                auto semaphore = semaphore_controller->getSemaphoreAt(next_pos);
                if (semaphore && !semaphore->isGreen()) {
                    return false;
                }
            }
            return true;
        }
    }
    
    // If next position is current position (start of path), we can advance
    if (next_pos == position_ && path_index_ == 0) {
        return true;
    }
    
    return false;
}


bool Vehicle::advance() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (path_index_ < current_path_.size()) {
        auto next_pos = current_path_[path_index_];
        
        if (city_) {
            auto next_intersection = city_->getIntersection(next_pos);
            auto current_intersection = city_->getIntersection(position_);
            
            // Si nos movemos a una intersección diferente
            if (next_pos != position_) {
                if (next_intersection) {
                    // 1. Intentar adquirir el lock de la intersección (mutex)
                    if (!next_intersection->try_lock()) {
                        return false; // La intersección fue ocupada en el último milisegundo
                    }
                }
                
                // 2. Libera bloqueo de la intersección anterior
                if (current_intersection) {
                    current_intersection->unlock();
                }
            } else if (path_index_ == 0) {
                // Already locked in setRoute(); nothing to do here.
                // The unlock happens on the next call when next_pos != position_.
            }

            auto street = city_->getStreet(position_, next_pos);
            if (street) {
                traffic_simulation::Logger::getInstance().info(
                    "Vehículo " + std::to_string(id_) + 
                    " transitando por " + street->getName());
            }
        }

        if (is_waiting_.load()) {
            auto wait_end = std::chrono::steady_clock::now();
            double added_time = std::chrono::duration<double>(wait_end - wait_start_).count();
            wait_time_.store(wait_time_.load() + added_time);

            is_waiting_ = false;
        }
        position_ = next_pos;
        ++path_index_;
        return true; // Éxito al avanzar
    }
    return false;
}


void Vehicle::waitAtIntersection() {
    std::unique_lock<std::mutex> lock(state_mutex_);
    cv_.wait(lock, [this] {
        return state_.load() == traffic_simulation::VehicleState::WAITING || 
               state_.load() == traffic_simulation::VehicleState::MOVING ||
               !running_.load();
    });
}


bool Vehicle::isArrived() const {
    return position_ == destination_;
}


traffic_simulation::VehicleState Vehicle::getState() const {
    return state_.load();
}


city::Coordinate Vehicle::getPosition() const {
    return position_;
}
city::Coordinate Vehicle::getDestination() const {
    return destination_;
}



city::Coordinate Vehicle::getNextPosition() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    // Si aún hay camino por recorrer, devolvemos la siguiente celda
    if (path_index_ < current_path_.size()) {
        return current_path_[path_index_];
    }
    // Si ya llegó, la siguiente posición es la actual
    return position_; 
}


int Vehicle::getId() const {
    return id_;
}


std::pair<double, double> Vehicle::getMetrics() const {
    return {total_travel_time_.load(), wait_time_.load()};
}


void Vehicle::startThread() {
    running_ = true;
    thread_ = std::thread([this]() { run(); });
}


void Vehicle::stopThread() {
    running_ = false;
    notifyConditionsChanged();
}


void Vehicle::joinThread() {
    if (thread_.joinable()) {
        thread_.join();
    }
}


void Vehicle::notifyConditionsChanged() {
    conditions_changed_ = true;
    cv_.notify_all();
}


void Vehicle::setState(traffic_simulation::VehicleState new_state) {
    state_ = new_state;
}


std::vector<city::Coordinate> Vehicle::getRemainingPath() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    std::vector<city::Coordinate> path;
    // Extraemos solo el camino que le falta por recorrer
    for (std::size_t i = path_index_; i < current_path_.size(); ++i) {
        path.push_back(current_path_[i]);
    }
    return path;
}


void Vehicle::setSemaphoreController(const traffic::SemaphoreController* controller) {
    semaphore_controller_ = controller;
}


void vehicle::Vehicle::setMetricsCollector(city::monitoring::MetricsCollector* collector) {
    metrics_collector_ = collector;
}

} // namespace vehicle
