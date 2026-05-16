/**
 * @file pathfinder.cpp
 * @brief Implementation of A* pathfinding with OpenMP parallelization
 * @author Traffic Simulation Team
 * @version 1.0
 */

#include "vehicle/pathfinder.hpp"
#include "city/city.hpp"
#include "city/intersection.hpp"
#include "city/street.hpp"
#include <limits>
#include <algorithm>
#include <map>
#include <set>

namespace vehicle {

int Pathfinder::heuristic(const city::Coordinate& a,
                          const city::Coordinate& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::vector<city::Coordinate> Pathfinder::getNeighbors(
    const city::Coordinate& coord,
    const city::City& city,
    const city::Coordinate& avoid) {
    std::vector<city::Coordinate> neighbors;
    const std::pair<city::Coordinate, city::Direction> moves[] = {
        {{0, -1}, city::Direction::NORTH},
        {{0, 1},  city::Direction::SOUTH},
        {{1, 0},  city::Direction::EAST},
        {{-1, 0}, city::Direction::WEST}
    };

    for (const auto& [dir_coord, travel_dir] : moves) {
        auto neighbor_coord = coord + dir_coord;

        if (city.isValidCoordinate(neighbor_coord)) {
            auto street = city.getStreet(coord, neighbor_coord);
            if (street && street->canTravel(travel_dir)) {
                // NUEVO: Solo agregamos el vecino si NO es el nodo bloqueado
                if (neighbor_coord != avoid) {
                    neighbors.push_back(neighbor_coord);
                }
            }
        }
    }
    return neighbors;
}

std::vector<city::Coordinate> Pathfinder::findPath(
    const city::Coordinate& start,
    const city::Coordinate& goal,
    const city::City& city,
    const city::Coordinate& avoid){

    std::vector<city::Coordinate> path;

    // If start equals goal, return empty path
    if (start == goal) {
        return path;
    }

    // Priority queue for open list (min-heap by f_cost)
    std::priority_queue<Node> open_list;

    // For fast lookup of visited nodes
    std::set<city::Coordinate> closed_set;

    // Map to reconstruct path
    std::map<city::Coordinate, city::Coordinate> came_from;

    // Start node
    Node start_node;
    start_node.coord = start;
    start_node.g_cost = 0;
    start_node.h_cost = heuristic(start, goal);
    open_list.push(start_node);

    while (!open_list.empty()) {
        // Get node with lowest f_cost
        Node current = open_list.top();
        open_list.pop();

        // Check if we reached the goal
        if (current.coord == goal) {
            // Reconstruct path
            auto node = goal;
            while (node != start) {
                path.push_back(node);
                node = came_from[node];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        // Skip if already visited
        if (closed_set.find(current.coord) != closed_set.end()) {
            continue;
        }

        closed_set.insert(current.coord);

        // Explore neighbors
        auto neighbors = getNeighbors(current.coord, city, avoid);

        for (const auto& neighbor : neighbors) {
            // Skip if already visited
            if (closed_set.find(neighbor) != closed_set.end()) {
                continue;
            }

            int tentative_g = current.g_cost + 1; // Cost is 1 per step

            Node neighbor_node;
            neighbor_node.coord = neighbor;
            neighbor_node.g_cost = tentative_g;
            neighbor_node.h_cost = heuristic(neighbor, goal);

            // Check if this path is better
            bool is_better = true;

            // We need to iterate priority_queue - use a workaround
            // Copy to temp container to check
            std::vector<Node> temp_nodes;
            std::priority_queue<Node> temp_copy = open_list;
            while (!temp_copy.empty()) {
                temp_nodes.push_back(temp_copy.top());
                temp_copy.pop();
            }

            for (const auto& node : temp_nodes) {
                if (node.coord == neighbor && node.g_cost <= tentative_g) {
                    is_better = false;
                    break;
                }
            }

            if (is_better) {
                came_from[neighbor] = current.coord;
                open_list.push(neighbor_node);
            }
        }
    }

    // No path found
    return path;
}

void Pathfinder::findPathsParallel(
    const std::vector<std::tuple<city::Coordinate,
                                  city::Coordinate,
                                  int>>& requests,
    const city::City& city,
    std::vector<std::pair<int, std::vector<city::Coordinate>>>& results) {

    std::size_t num_requests = requests.size();

    // Resize results vector
    results.resize(num_requests);

    // Parallel pathfinding using OpenMP
    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(num_requests); ++i) {
        const auto& request = requests[i];
        const auto& start = std::get<0>(request);
        const auto& goal = std::get<1>(request);
        int vehicle_id = std::get<2>(request);

        // Find path using A*
        std::vector<city::Coordinate> path = findPath(start, goal, city);

        // Store result
        results[i] = {vehicle_id, std::move(path)};
    }
}

} // namespace vehicle
