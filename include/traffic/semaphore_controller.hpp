#ifndef TRAFFIC_SEMAPHORE_CONTROLLER_HPP
#define TRAFFIC_SEMAPHORE_CONTROLLER_HPP

#include "semaphore.hpp"
#include "city/intersection.hpp"

#include <vector>
#include <unordered_map>
#include <memory>
#include <cstddef>

namespace city {
    class City;
}

namespace traffic {

/**
 * @brief Controller that manages all traffic light semaphores in the city
 * 
 * Handles initialization, coordination, and timing updates for all
 * traffic lights across the city grid.
 */
class SemaphoreController {
public:
    /**
     * @brief Construct a new SemaphoreController
     */
    SemaphoreController();

    ~SemaphoreController();

    // Non-copyable, non-movable
    SemaphoreController(const SemaphoreController&) = delete;
    SemaphoreController& operator=(const SemaphoreController&) = delete;
    SemaphoreController(SemaphoreController&&) = delete;
    SemaphoreController& operator=(SemaphoreController&&) = delete;

    /**
     * @brief Initialize semaphores at main intersections in the city
     * @param city Reference to the City object
     * @param num_semaphores Number of semaphores to create
     */
    void initialize(city::City& city, int num_semaphores, int green = 5, int yellow = 2, int red = 6);
    
    /**
     * @brief Get the semaphore at a specific coordinate
     * @param coord Coordinate to look up (using city::Coordinate)
     * @return Pointer to Semaphore if found, nullptr otherwise
     */
    Semaphore* getSemaphoreAt(const city::Coordinate& coord);
    
    /**
     * @brief Get the semaphore at a specific coordinate (const version)
     * @param coord Coordinate to look up (using city::Coordinate)
     * @return Pointer to Semaphore if found, nullptr otherwise
     */
    const Semaphore* getSemaphoreAt(const city::Coordinate& coord) const;

    /**
     * @brief Stop all semaphore threads
     */
    void stopAll();

    /**
     * @brief Update timing for all semaphores
     * @param green Green duration in seconds
     * @param yellow Yellow duration in seconds
     * @param red Red duration in seconds
     */
    void setTiming(int green, int yellow, int red);

    /**
     * @brief Get the number of semaphores managed
     * @return Number of semaphores
     */
    std::size_t getSemaphoreCount() const;

private:
    std::vector<std::unique_ptr<Semaphore>> semaphores_;
    std::unordered_map<city::Coordinate, Semaphore*, std::hash<city::Coordinate>> semaphore_map_;
};

} // namespace traffic

#endif // TRAFFIC_SEMAPHORE_CONTROLLER_HPP
