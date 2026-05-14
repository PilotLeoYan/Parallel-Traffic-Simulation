/**
 * @file pathfinder.hpp
 * @brief A* pathfinding implementation for city traffic simulation
 * @author Traffic Simulation Team
 * @version 1.0
 */

#pragma once

#include "city/city.hpp"
#include "city/intersection.hpp"
#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <functional>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace vehicle {

/**
 * @brief A* pathfinding implementation for grid-based navigation
 * 
 * Uses Manhattan distance as heuristic and supports parallel pathfinding
 * via OpenMP for multiple vehicles.
 */
class Pathfinder {
public:
    /**
     * @brief Node structure for A* algorithm
     */
    struct Node {
        city::Coordinate coord;
        int g_cost;  // Cost from start
        int h_cost;  // Heuristic cost to goal
        int f_cost() const { return g_cost + h_cost; }
        
        bool operator<(const Node& other) const {
            return f_cost() > other.f_cost(); // Reverse for min-heap
        }
    };

    /**
     * @brief Find a path using A* algorithm
     * @param start Starting coordinate
     * @param goal Goal coordinate
     * @param city Reference to the city for neighbor lookup
     * @return Vector of coordinates representing the path, empty if no path found
     */
    static std::vector<city::Coordinate> findPath(
        const city::Coordinate& start,
        const city::Coordinate& goal,
        const city::City& city);

    /**
     * @brief Find paths for multiple vehicles in parallel using OpenMP
     * @param requests Vector of (start, goal, vehicle_id) tuples
     * @param city Reference to the city
     * @param results Map to store results (vehicle_id -> path)
     */
    static void findPathsParallel(
        const std::vector<std::tuple<city::Coordinate,
                                      city::Coordinate,
                                      int>>& requests,
        const city::City& city,
        std::vector<std::pair<int, std::vector<city::Coordinate>>>& results);

private:
    /**
     * @brief Calculate Manhattan distance heuristic
     */
    static int heuristic(const city::Coordinate& a,
                         const city::Coordinate& b);

    /**
     * @brief Get valid neighboring coordinates (4-directional)
     */
    static std::vector<city::Coordinate> getNeighbors(
        const city::Coordinate& coord,
        const city::City& city);
};

} // namespace vehicle
