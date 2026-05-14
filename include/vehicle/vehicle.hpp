/**
 * @file vehicle.hpp
 * @brief Vehicle class for city traffic simulation
 * @author Traffic Simulation Team
 * @version 1.0
 */

#pragma once

#include "city/intersection.hpp"
#include "traffic/semaphore.hpp"
#include "traffic/semaphore_controller.hpp"
#include "common/types.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <atomic>
#include <chrono>

namespace city {
    class City;
}

namespace vehicle {

/**
 * @brief Represents a vehicle in the traffic simulation
 * 
 * Each vehicle runs on its own thread and navigates through
 * the city using A* pathfinding to reach its destination.
 */
class Vehicle {
public:
    /**
     * @brief Construct a new Vehicle
     * @param id Unique identifier for this vehicle
     */
    explicit Vehicle(int id);

    ~Vehicle();

    // Non-copyable, non-movable
    Vehicle(const Vehicle&) = delete;
    Vehicle& operator=(const Vehicle&) = delete;
    Vehicle(Vehicle&&) = delete;
    Vehicle& operator=(Vehicle&&) = delete;

    /**
     * @brief Set the route for this vehicle using A* pathfinding
     * @param start Starting coordinate
     * @param dest Destination coordinate
     * @param city Reference to the city for pathfinding
     * @return true if path was found, false otherwise
     */
    bool setRoute(const city::Coordinate& start,
                  const city::Coordinate& dest,
                  city::City& city);

    /**
     * @brief Main vehicle thread loop
     */
    void run();

    /**
     * @brief Check if the vehicle can advance to the next position
     * @param city Reference to the city
     * @param semaphore_controller Reference to semaphore controller for light state check
     * @return true if vehicle can advance, false otherwise
     */
    bool canAdvance(city::City& city, const traffic::SemaphoreController* semaphore_controller);

    /**
     * @brief Move the vehicle to the next position in the path
     */
    void advance();

    /**
     * @brief Block the vehicle until it can proceed
     */
    void waitAtIntersection();

    /**
     * @brief Check if the vehicle has arrived at its destination
     * @return true if vehicle is at destination
     */
    bool isArrived() const;

    /**
     * @brief Get the vehicle's current state
     * @return Current VehicleState
     */
    traffic_simulation::VehicleState getState() const;

    /**
     * @brief Get the vehicle's current position
     * @return Current coordinate
     */
    city::Coordinate getPosition() const;

    /**
     * @brief Get the vehicle's next position in the path
     * @return Next coordinate, or current position if at destination
     */
    city::Coordinate getNextPosition() const;

    /**
     * @brief Get the vehicle's ID
     * @return Vehicle ID
     */
    int getId() const;

    /**
     * @brief Get metrics for this vehicle
     * @return Pair of (travel_time, wait_time) in seconds
     */
    std::pair<double, double> getMetrics() const;

    /**
     * @brief Obtiene el camino restante para dibujarlo
     */
    std::vector<city::Coordinate> getRemainingPath() const;

    /**
     * @brief Start the vehicle thread
     */
    void startThread();

    /**
     * @brief Stop the vehicle thread
     */
    void stopThread();

    /**
     * @brief Wait for the vehicle thread to finish
     */
    void joinThread();

    /**
     * @brief Notify the vehicle that conditions have changed (for CV)
     */
    void notifyConditionsChanged();

    /**
     * @brief Set the SemaphoreController reference for this vehicle
     * @param controller Pointer to the SemaphoreController
     */
    void setSemaphoreController(const traffic::SemaphoreController* controller);

private:
    /**
     * @brief Internal method to update vehicle state
     * @param new_state New state to set
     */
    void setState(traffic_simulation::VehicleState new_state);

    int id_;
    city::Coordinate position_;
    city::Coordinate destination_;
    std::vector<city::Coordinate> current_path_;
    std::atomic<traffic_simulation::VehicleState> state_;
    std::atomic<std::size_t> path_index_;

    // City pointer for runtime access
    city::City* city_;
    
    // Timing metrics
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point arrival_time_;
    std::atomic<double> total_travel_time_;
    std::atomic<double> wait_time_;
    std::chrono::steady_clock::time_point wait_start_;
    std::atomic<bool> is_waiting_;

    // Thread synchronization
    std::thread thread_;
    mutable std::mutex state_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_;
    std::atomic<bool> conditions_changed_;

    // Reference to the SemaphoreController for checking light states
    const traffic::SemaphoreController* semaphore_controller_ = nullptr;
};

} // namespace vehicle
