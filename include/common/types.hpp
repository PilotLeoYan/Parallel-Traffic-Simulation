/**
 * @file types.hpp
 * @brief Common types for the city traffic simulation
 * @author Traffic Simulation Team
 * @version 1.0
 */

#pragma once

#include <cstdint>
#include <chrono>

namespace traffic_simulation {

/**
 * @brief Represents a 2D coordinate on the simulation grid
 */
struct Coordinate {
    int x;
    int y;

    constexpr Coordinate() : x(0), y(0) {}
    constexpr Coordinate(int x, int y) : x(x), y(y) {}

    bool operator==(const Coordinate& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Coordinate& other) const {
        return !(*this == other);
    }

    Coordinate operator+(const Coordinate& other) const {
        return Coordinate(x + other.x, y + other.y);
    }
};

/**
 * @brief Cardinal directions for vehicle movement
 */
enum class Direction : uint8_t {
    NORTH = 0,
    SOUTH = 1,
    EAST  = 2,
    WEST  = 3
};

/**
 * @brief Traffic light states
 */
enum class TrafficLightState : uint8_t {
    GREEN  = 0,
    YELLOW = 1,
    RED    = 2
};

/**
 * @brief Vehicle states in the simulation
 */
enum class VehicleState : uint8_t {
    WAITING  = 0,
    MOVING   = 1,
    ARRIVED  = 2,
    BLOCKED  = 3
};

/**
 * @brief Configuration for simulation parameters
 */
struct SimulationConfig {
    int grid_size{12};
    int vehicle_count{10};
    int green_duration{5};
    int yellow_duration{2};
    int red_duration{6};
    int cell_size{60};  // Cell size in pixels

    SimulationConfig() = default;

    SimulationConfig(int grid_size, int vehicle_count, 
                      int green_duration, int yellow_duration, int red_duration)
        : grid_size(grid_size),
          vehicle_count(vehicle_count),
          green_duration(green_duration),
          yellow_duration(yellow_duration),
          red_duration(red_duration) {}
};

} // namespace traffic_simulation
