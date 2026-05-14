/**
 * @file vehicle_manager.hpp
 * @brief Vehicle manager for city traffic simulation
 * @author Traffic Simulation Team
 * @version 1.0
 */

#pragma once

#include "vehicle.hpp"
#include "pathfinder.hpp"
#include "city/city.hpp"
#include "monitoring/metrics.hpp"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <algorithm>
#include "gui/simulation_window.hpp"

namespace vehicle {

/**
 * @brief Manages all vehicles in the simulation
 * 
 * Responsible for spawning vehicles, coordinating their activities,
 * and collecting aggregate metrics.
 */
class VehicleManager {
public:
    /**
     * @brief Construct a new VehicleManager
     * @param city Reference to the city
     */
    explicit VehicleManager(const city::City& city);

    ~VehicleManager();

    // Non-copyable, non-movable
    VehicleManager(const VehicleManager&) = delete;
    VehicleManager& operator=(const VehicleManager&) = delete;
    VehicleManager(VehicleManager&&) = delete;
    VehicleManager& operator=(VehicleManager&&) = delete;

    /**
     * @brief Initialize and spawn vehicles
     * @param vehicle_count Number of vehicles to create
     */
    void initialize(int vehicle_count);

    /**
     * @brief Update all vehicles (called each simulation tick)
     */
    void updateAll();

    /**
     * @brief Get aggregate metrics from all vehicles
     * @return Vector of vehicle metrics
     */
    std::vector<city::monitoring::VehicleMetrics> getMetrics() const;

    /**
     * @brief Wait for all vehicles to arrive at their destinations
     */
    void waitForCompletion();

    /**
     * @brief Get the number of active (non-arrived) vehicles
     * @return Count of active vehicles
     */
    std::size_t getActiveVehicleCount() const;

    /**
     * @brief Get total number of vehicles
     * @return Total vehicle count
     */
    std::size_t getTotalVehicleCount() const;

    /**
     * @brief Get render information for all vehicles (for GUI)
     * @return Vector of VehicleRenderInfo
     */
    std::vector<gui::VehicleRenderInfo> getRenderInfo() const;

    /**
     * @brief Stop all vehicle threads
     */
    void shutdown();

    /**
     * @brief Get random coordinate from valid intersections
     * @return Random coordinate
     */
    city::Coordinate getRandomCoordinate() const;

    /**
     * @brief Generate a random destination different from start
     */
    city::Coordinate generateDestination(const city::Coordinate& start) const;

    /**
     * @brief Notify vehicles that conditions have changed (e.g., traffic light state)
     */
    void setSemaphoreController(const traffic::SemaphoreController* controller);

private:
    const city::City& city_;
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
    std::atomic<std::size_t> arrived_count_;
    std::atomic<std::size_t> total_vehicles_;
    std::atomic<bool> simulation_complete_;

    // Random number generation
    mutable std::mt19937 rng_;
    mutable std::uniform_int_distribution<int> coord_dist_;

    // Reference to the SemaphoreController for notifying vehicles
    const traffic::SemaphoreController* semaphore_controller_ = nullptr;
};

} // namespace vehicle
