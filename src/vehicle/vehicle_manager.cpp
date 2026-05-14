/**
 * @file vehicle_manager.cpp
 * @brief Implementation of VehicleManager class
 * @author Traffic Simulation Team
 * @version 1.0
 */

#include "vehicle/vehicle_manager.hpp"
#include "vehicle/pathfinder.hpp"

namespace vehicle {

VehicleManager::VehicleManager(const city::City& city)
    : city_(city),
      arrived_count_(0),
      total_vehicles_(0),
      simulation_complete_(false),
      rng_(std::random_device{}()),
      coord_dist_(0, city.getGridSize() - 1) {
}

VehicleManager::~VehicleManager() {
    shutdown();
}

void VehicleManager::initialize(int vehicle_count) {
    vehicles_.clear();
    arrived_count_ = 0;
    total_vehicles_ = vehicle_count;
    simulation_complete_ = false;
    
    // Reserve space for vehicles
    vehicles_.reserve(vehicle_count);
    
    // Create vehicles with random start and destination
    for (int i = 0; i < vehicle_count; ++i) {
        auto vehicle = std::make_unique<Vehicle>(i);
        
        // Get random start and destination coordinates
        auto start = getRandomCoordinate();
        auto dest = generateDestination(start);
        
        // Set route (this also initializes vehicle position)
        vehicle->setRoute(start, dest, const_cast<city::City&>(city_));
        
        vehicle->setSemaphoreController(semaphore_controller_);
        
        // Start the vehicle thread
        vehicle->startThread();
        
        vehicles_.push_back(std::move(vehicle));
    }
}

void VehicleManager::updateAll() {
    // Notify all waiting vehicles that conditions may have changed
    for (auto& vehicle : vehicles_) {
        if (vehicle->getState() == traffic_simulation::VehicleState::WAITING ||
            vehicle->getState() == traffic_simulation::VehicleState::BLOCKED) {
            vehicle->notifyConditionsChanged();
        }
    }
}

std::vector<city::monitoring::VehicleMetrics> VehicleManager::getMetrics() const {
    std::vector<city::monitoring::VehicleMetrics> metrics;
    metrics.reserve(vehicles_.size());
    
    std::size_t arrival_order = 0;
    for (const auto& vehicle : vehicles_) {
        city::monitoring::VehicleMetrics m;
        m.id = vehicle->getId();
        
        auto [travel_time, wait_time] = vehicle->getMetrics();
        m.travel_time = travel_time;
        m.wait_time = wait_time;
        
        if (travel_time > 0) {
            m.wait_percentage = (wait_time / travel_time) * 100.0;
        } else {
            m.wait_percentage = 0.0;
        }
        
        if (vehicle->getState() == traffic_simulation::VehicleState::ARRIVED) {
            m.arrival_order = ++arrival_order;
        }
        
        metrics.push_back(m);
    }
    
    return metrics;
}

void VehicleManager::waitForCompletion() {
    for (auto& vehicle : vehicles_) {
        vehicle->joinThread();
    }
    simulation_complete_ = true;
}

std::size_t VehicleManager::getActiveVehicleCount() const {
    std::size_t active = 0;
    for (const auto& vehicle : vehicles_) {
        if (vehicle->getState() != traffic_simulation::VehicleState::ARRIVED) {
            ++active;
        }
    }
    return active;
}

std::size_t VehicleManager::getTotalVehicleCount() const {
    return total_vehicles_.load();
}

std::vector<gui::VehicleRenderInfo> VehicleManager::getRenderInfo() const {
    std::vector<gui::VehicleRenderInfo> render_list;
    render_list.reserve(vehicles_.size());
    
    for (const auto& vehicle : vehicles_) {
        city::Coordinate current = vehicle->getPosition();
        city::Coordinate next = vehicle->getNextPosition();
        
        // AQUÍ COLOCAMOS TU FÓRMULA
        float dx = next.x - current.x;
        float dy = next.y - current.y;
        float angle = 0.0f;
        
        // Si el vehículo se está moviendo o tiene una dirección clara
        if (dx != 0 || dy != 0) {
            angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
        }
        
        // Creamos la estructura visual con el ángulo
        gui::VehicleRenderInfo info(
            vehicle->getId(), 
            current.x, current.y, 
            current.x, current.y,
            angle
        );
        
        info.is_waiting = (vehicle->getState() == traffic_simulation::VehicleState::WAITING || 
                           vehicle->getState() == traffic_simulation::VehicleState::BLOCKED);
                           
        info.remaining_path = vehicle->getRemainingPath();
        
        render_list.push_back(info);
    }
    
    return render_list;
}

void VehicleManager::shutdown() {
    for (auto& vehicle : vehicles_) {
        vehicle->stopThread();
        vehicle->joinThread();
    }
    vehicles_.clear();
    simulation_complete_ = true;
}

city::Coordinate VehicleManager::getRandomCoordinate() const {
    int x = coord_dist_(rng_);
    int y = coord_dist_(rng_);
    return city::Coordinate{x, y};
}

city::Coordinate VehicleManager::generateDestination(
    const city::Coordinate& start) const {
    
    city::Coordinate dest;
    
    // Keep generating until we get a different coordinate
    do {
        dest = getRandomCoordinate();
    } while (dest == start && city_.getGridSize() > 1);
    
    return dest;
}

void VehicleManager::setSemaphoreController(const traffic::SemaphoreController* controller) {
    semaphore_controller_ = controller;
}

} // namespace vehicle
