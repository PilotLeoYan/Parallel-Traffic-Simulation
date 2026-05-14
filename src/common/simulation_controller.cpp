#include "common/simulation_controller.hpp"

#include <stdexcept>
#include <iostream>
#include <chrono>
#include <cmath>

namespace traffic_simulation {

SimulationController::SimulationController()
    : vehicle_manager_(nullptr),
      is_running_(false),
      is_paused_(false),
      simulation_complete_(false),
      speed_multiplier_(1.0) {
}

SimulationController::~SimulationController() {
    if (simulation_thread_.joinable()) {
        is_running_ = false;
        simulation_thread_.join();
    }
    if (vehicle_manager_) {
        vehicle_manager_->shutdown();
        delete vehicle_manager_;
    }
}

void SimulationController::initialize(const SimulationConfig& config) {
    Logger::getInstance().info("Initializing simulation with config: grid_size=" + 
                               std::to_string(config.grid_size) +
                               ", vehicles=" + std::to_string(config.vehicle_count));

    config_ = config;
    simulation_complete_ = false;
    is_paused_ = false;
    speed_multiplier_ = 1.0;

    // Initialize the city
    city_.initialize(config.grid_size);
    Logger::getInstance().info("City initialized with grid size " + std::to_string(config.grid_size));

    // Initialize semaphores at key intersections


    // Vamos a poner semáforos en la mitad de las intersecciones disponibles
    //int num_semaphores = (config.grid_size * config.grid_size) / 2;

    // O si quieres que TODAS las intersecciones tengan semáforo, usa esto:
    int num_semaphores = config.grid_size * config.grid_size;

    semaphore_controller_.initialize(city_, num_semaphores, 
                                     config.green_duration, 
                                     config.yellow_duration, 
                                     config.red_duration);
    Logger::getInstance().info("Semaphore controller initialized with " + 
                               std::to_string(semaphore_controller_.getSemaphoreCount()) + " semaphores");

    // Create and initialize vehicle manager
    vehicle_manager_ = new vehicle::VehicleManager(city_);
    
    vehicle_manager_->setSemaphoreController(&semaphore_controller_);

    vehicle_manager_->initialize(config.vehicle_count);
    Logger::getInstance().info("Vehicle manager initialized with " + 
                               std::to_string(vehicle_manager_->getTotalVehicleCount()) + " vehicles");

    Logger::getInstance().info("Simulation initialization complete");
}

int SimulationController::run() {
    if (!city_.isInitialized()) {
        Logger::getInstance().error("Simulation not initialized. Call initialize() first.");
        return 1;
    }

    if (is_running_) {
        Logger::getInstance().warning("Simulation already running");
        return 1;
    }

    Logger::getInstance().info("Starting simulation...");
    is_running_ = true;
    simulation_complete_ = false;

    // Start the simulation loop in a separate thread
    simulation_thread_ = std::thread(&SimulationController::simulationLoop, this);

    return 0;
}

void SimulationController::simulationLoop() {
    Logger::getInstance().info("Simulation loop started");

    // Calculate tick interval based on speed multiplier
    const int base_tick_ms = constants::TICK_DURATION_MS;
    last_tick_time_ = std::chrono::steady_clock::now();

    while (is_running_) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - last_tick_time_).count();

        // Calculate required sleep time based on speed multiplier
        int tick_interval = static_cast<int>(base_tick_ms / speed_multiplier_);

        if (elapsed_ms >= tick_interval) {
            if (!is_paused_ && !simulation_complete_) {
                processTick();
                last_tick_time_ = current_time;
            } else {
                // Even when paused or complete, we need to sleep to avoid busy-waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } else {
            // Sleep for a short time to avoid busy-waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Check if simulation is complete (all vehicles arrived)
        if (!simulation_complete_ && vehicle_manager_ != nullptr) {
            if (vehicle_manager_->getActiveVehicleCount() == 0) {
                simulation_complete_ = true;
                Logger::getInstance().info("All vehicles have arrived. Simulation complete.");
            }
        }
    }

    Logger::getInstance().info("Simulation loop ended");
}

void SimulationController::processTick() {
    if (vehicle_manager_ != nullptr) {
        vehicle_manager_->updateAll();
    }
}

void SimulationController::pause() {
    if (is_paused_) {
        return;
    }
    is_paused_ = true;
    Logger::getInstance().info("Simulation paused");
}

void SimulationController::resume() {
    if (!is_paused_) {
        return;
    }
    is_paused_ = false;
    last_tick_time_ = std::chrono::steady_clock::now();
    Logger::getInstance().info("Simulation resumed");
}

void SimulationController::setSpeed(double speed) {
    if (speed <= 0.0) {
        Logger::getInstance().warning("Invalid speed value, ignoring");
        return;
    }
    speed_multiplier_ = speed;
    Logger::getInstance().info("Simulation speed set to " + std::to_string(speed) + "x");
}

void SimulationController::reset() {
    Logger::getInstance().info("Resetting simulation...");

    // Stop the simulation thread if running
    bool was_running = is_running_;
    if (simulation_thread_.joinable()) {
        is_running_ = false;
        simulation_thread_.join();
    }

    // Clean up vehicle manager
    if (vehicle_manager_) {
        vehicle_manager_->shutdown();
        delete vehicle_manager_;
        vehicle_manager_ = nullptr;
    }

    // Re-initialize with the same config
    initialize(config_);

    if (was_running) {
        run();
    }
}

city::monitoring::MetricsCollector& SimulationController::getMetrics() {
    return metrics_collector_;
}

vehicle::VehicleManager& SimulationController::getVehicleManager() {
    if (!vehicle_manager_) {
        throw std::runtime_error("VehicleManager not initialized");
    }
    return *vehicle_manager_;
}

traffic::SemaphoreController& SimulationController::getSemaphoreController() {
    return semaphore_controller_;
}

city::City& SimulationController::getCity() {
    return city_;
}

bool SimulationController::isRunning() const {
    return is_running_;
}

bool SimulationController::isPaused() const {
    return is_paused_;
}

double SimulationController::getSpeed() const {
    return speed_multiplier_;
}

SimulationConfig SimulationController::getConfig() const {
    return config_;
}

} // namespace traffic_simulation
