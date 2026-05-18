/**
 * @file simulation_controller.hpp
 * @brief Simulation controller for orchestrating all traffic simulation subsystems
 */

#pragma once

#include "common/types.hpp"
#include "common/logger.hpp"
#include "common/constants.hpp"
#include "city/city.hpp"
#include "traffic/semaphore_controller.hpp"
#include "vehicle/vehicle_manager.hpp"
#include "monitoring/metrics_collector.hpp"

#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

namespace traffic_simulation {

class SimulationController {
public:
    SimulationController();
    ~SimulationController();

    SimulationController(const SimulationController&) = delete;
    SimulationController& operator=(const SimulationController&) = delete;
    SimulationController(SimulationController&&) = delete;
    SimulationController& operator=(SimulationController&&) = delete;

    void initialize(const SimulationConfig& config);

    int run();

    void pause();

    void resume();

    /**
     * @brief Advance simulation by exactly one tick.
     * Auto-pauses the loop if it is currently running, then
     * processes a single tick so the user can step frame by frame.
     */
    void stepOnce();

    void setSpeed(double speed);

    void reset();

    city::monitoring::MetricsCollector& getMetrics();

    vehicle::VehicleManager& getVehicleManager();

    traffic::SemaphoreController& getSemaphoreController();

    city::City& getCity();

    bool isRunning() const;

    bool isPaused() const;

    double getSpeed() const;

    SimulationConfig getConfig() const;

private:
    void simulationLoop();
    void processTick();

    city::City city_;
    traffic::SemaphoreController semaphore_controller_;
    vehicle::VehicleManager* vehicle_manager_;
    city::monitoring::MetricsCollector metrics_collector_;

    SimulationConfig config_;
    std::atomic<bool> is_running_;
    std::atomic<bool> is_paused_;
    std::atomic<bool> vehicles_paused_{ false };
    std::atomic<bool> simulation_complete_;
    std::atomic<double> speed_multiplier_;

    std::thread simulation_thread_;
    std::chrono::steady_clock::time_point last_tick_time_;
};

} // namespace traffic_simulation
