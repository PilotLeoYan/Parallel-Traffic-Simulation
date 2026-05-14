#include "traffic/semaphore_controller.hpp"

#include "city/city.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <functional>

namespace traffic {

SemaphoreController::SemaphoreController() = default;

SemaphoreController::~SemaphoreController() {
    stopAll();
}

    void SemaphoreController::initialize(city::City& city, int num_semaphores, int green, int yellow, int red) {
    if (!city.isInitialized()) {
        throw std::runtime_error("City must be initialized before semaphore controller");
    }

    if (num_semaphores <= 0) {
        throw std::invalid_argument("Number of semaphores must be positive");
    }

    // Clear any existing semaphores
    stopAll();
    semaphores_.clear();
    semaphore_map_.clear();

    const int grid_size = city.getGridSize();
    
    // Calculate spacing between semaphores to distribute them evenly
    // We want to cover the grid meaningfully, so we take sqrt(num_semaphores)
    // per axis and distribute evenly
    int semaphores_per_axis = static_cast<int>(std::sqrt(static_cast<double>(num_semaphores)));
    if (semaphores_per_axis < 1) {
        semaphores_per_axis = 1;
    }

    // Calculate step size to distribute semaphores across the grid
    // Leave margin at edges (start from 1 to grid_size-2 to avoid corner issues)
    const int start = 1;
    const int end = grid_size - 1;
    const int range = end - start;
    
    if (range <= 0) {
        // Grid too small, just put semaphores at available intersections
        for (int i = start; i <= end && static_cast<int>(semaphores_.size()) < num_semaphores; ++i) {
            city::Coordinate coord{i, i};
            auto intersection = city.getIntersection(coord);
            if (intersection) {
                auto semaphore = std::make_unique<Semaphore>(intersection->getId(), green, yellow, red);
                semaphore->start();
                
                semaphores_.push_back(std::move(semaphore));
                semaphore_map_[coord] = semaphores_.back().get();
            }
        }
        return;
    }

    int step = range / std::max(1, semaphores_per_axis - 1);
    if (step <= 0) {
        step = 1;
    }

    // Collect candidate coordinates
    std::vector<city::Coordinate> candidates;
    for (int x = start; x <= end; x += step) {
        for (int y = start; y <= end; y += step) {
            candidates.emplace_back(city::Coordinate{x, y});
            if (static_cast<int>(candidates.size()) >= num_semaphores) {
                break;
            }
        }
        if (static_cast<int>(candidates.size()) >= num_semaphores) {
            break;
        }
    }

    // Create semaphores at selected coordinates
    for (const auto& coord : candidates) {
        auto intersection = city.getIntersection(coord);
        if (intersection) {
            auto semaphore = std::make_unique<Semaphore>(intersection->getId(), green, yellow, red);
            semaphore->start();
            
            semaphores_.push_back(std::move(semaphore));
            semaphore_map_[coord] = semaphores_.back().get();
        }
    }
}

Semaphore* SemaphoreController::getSemaphoreAt(const city::Coordinate& coord) {
    auto it = semaphore_map_.find(coord);
    if (it != semaphore_map_.end()) {
        return it->second;
    }
    return nullptr;
}

const Semaphore* SemaphoreController::getSemaphoreAt(const city::Coordinate& coord) const {
    auto it = semaphore_map_.find(coord);
    if (it != semaphore_map_.end()) {
        return it->second;
    }
    return nullptr;
}

void SemaphoreController::stopAll() {
    for (auto& semaphore : semaphores_) {
        semaphore->stop();
    }
}

void SemaphoreController::setTiming(int green, int yellow, int red) {
    // Note: This is a placeholder for future timing update functionality
    // Currently semaphores use their initial timing values
    (void)green;
    (void)yellow;
    (void)red;
}

std::size_t SemaphoreController::getSemaphoreCount() const {
    return semaphores_.size();
}

} // namespace traffic
