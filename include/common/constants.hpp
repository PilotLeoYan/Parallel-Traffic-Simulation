/**
 * @file constants.hpp
 * @brief Constants for the city traffic simulation
 * @author Traffic Simulation Team
 * @version 1.0
 */

#pragma once

namespace traffic_simulation {
namespace constants {

/**
 * @name Grid Dimensions
 * @{
 */
constexpr int MIN_GRID_SIZE = 12;      ///< Minimum allowed grid size
constexpr int DEFAULT_GRID_SIZE = 12;  ///< Default grid size for simulation
constexpr int MAX_GRID_SIZE = 100;     ///< Maximum allowed grid size
/** @} */

/**
 * @name Timing Constants
 * @{
 */
constexpr int DEFAULT_GREEN_DURATION = 5;   ///< Default green light duration (seconds)
constexpr int DEFAULT_YELLOW_DURATION = 2;  ///< Default yellow light duration (seconds)
constexpr int DEFAULT_RED_DURATION = 6;      ///< Default red light duration (seconds)
constexpr int TICK_DURATION_MS = 1000;        ///< Simulation tick duration in milliseconds
/** @} */

/**
 * @name Vehicle Constants
 * @{
 */
constexpr double DEFAULT_VEHICLE_SPEED = 1.0;   ///< Default vehicle speed (cells per second)
constexpr int MIN_VEHICLE_COUNT = 1;            ///< Minimum number of vehicles
constexpr int MAX_VEHICLE_COUNT = 100;          ///< Maximum number of vehicles
constexpr int DEFAULT_VEHICLE_COUNT = 10;       ///< Default number of vehicles
/** @} */

/**
 * @name Intersection Constants
 * @{
 */
constexpr int INTERSECTION_SIZE = 2;  ///< Size of an intersection in grid cells
/** @} */

} // namespace constants
} // namespace traffic_simulation
