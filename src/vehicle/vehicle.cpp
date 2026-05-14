/**
 * @file vehicle.cpp
 * @brief Implementation of Vehicle class
 * @author Traffic Simulation Team
 * @version 1.0
 */

#include "vehicle/vehicle.hpp"
#include "vehicle/pathfinder.hpp"
#include <cmath>

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
    
    state_ = traffic_simulation::VehicleState::WAITING;
    return true;
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
            
            // Check if we can advance
            if (city_ && canAdvance(*city_, semaphore_controller_)) {
                advance();
            } else {
                // Record wait time
                if (!is_waiting_.load()) {
                    wait_start_ = std::chrono::steady_clock::now();
                    is_waiting_ = true;
                }
                
                // Block using condition variable
                std::unique_lock<std::mutex> lock(state_mutex_);
                cv_.wait_for(lock, std::chrono::milliseconds(100), [this] {
                    return conditions_changed_.load() || !running_.load();
                });
                conditions_changed_ = false;
                
                // Accumulate wait time
                if (is_waiting_.load()) {
                    auto wait_duration = std::chrono::steady_clock::now() - wait_start_;
                    wait_time_.store(wait_time_.load() + std::chrono::duration<double>(wait_duration).count());
                    is_waiting_ = false;
                }
            }
        }
        else if (current_state == traffic_simulation::VehicleState::MOVING) {
            // Simulate movement time
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            // After moving, set to waiting for next intersection
            setState(traffic_simulation::VehicleState::WAITING);
        }
        
        // Check if arrived
        if (isArrived()) {
            arrival_time_ = std::chrono::steady_clock::now();
            auto travel_duration = arrival_time_ - start_time_;
            total_travel_time_ = std::chrono::duration<double>(travel_duration).count();
            setState(traffic_simulation::VehicleState::ARRIVED);
            break;
        }
    }
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

void Vehicle::advance() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (path_index_ < current_path_.size()) {
        position_ = current_path_[path_index_];
        ++path_index_;
        setState(traffic_simulation::VehicleState::MOVING);
    }
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

} // namespace vehicle
