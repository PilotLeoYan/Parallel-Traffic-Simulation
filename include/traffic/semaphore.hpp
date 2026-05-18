#ifndef TRAFFIC_SEMAPHORE_HPP
#define TRAFFIC_SEMAPHORE_HPP

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <cstddef>

namespace traffic {

/**
 * @brief Represents the state of a traffic light
 */
enum class TrafficLightState {
    GREEN,
    YELLOW,
    RED
};

/**
 * @brief Traffic light semaphore that controls vehicle flow at intersections
 * 
 * Each Semaphore runs on an independent thread, cycling through states:
 * Green (default 5s) -> Yellow (default 2s) -> Red (default 6s) -> repeat
 */
class Semaphore {
public:
    /**
     * @brief Construct a new Semaphore
     * @param intersection_id Unique identifier for this intersection
     * @param green_duration Duration of green light in seconds
     * @param yellow_duration Duration of yellow light in seconds
     * @param red_duration Duration of red light in seconds
     */
    Semaphore(std::size_t intersection_id,
              int green_duration = 5,
              int yellow_duration = 2,
              int red_duration = 6);

    ~Semaphore();

    // Non-copyable, non-movable
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    Semaphore& operator=(Semaphore&&) = delete;

    /**
     * @brief Start the semaphore thread loop
     */
    void start();

    /**
     * @brief Stop the semaphore thread
     */
    void stop();

    /**
     * @brief Get the current state of the traffic light
     * @return Current TrafficLightState
     */
    TrafficLightState getState() const;

    /**
     * @brief Check if the light is green or yellow (vehicles can proceed)
     * @return true if state is GREEN or YELLOW
     */
    bool isGreen() const;

    /**
     * @brief Block until the light is green or yellow
     */
    void waitForGreen() const;

    /**
     * @brief Get the intersection ID
     * @return Intersection identifier
     */
    std::size_t getIntersectionId() const;

    /**
     * @brief Pause the semaphore cycle (pause animation)
     */
    void pauseCycle();

    /**
     * @brief Resume the semaphore cycle
     */
    void resumeCycle();
private:
    /**
     * @brief Internal state machine loop that cycles through traffic light states
     */
    void cycle();

    std::size_t intersection_id_;
    TrafficLightState state_;
    
    int green_duration_;
    int yellow_duration_;
    int red_duration_;
    
    mutable std::shared_mutex rw_mutex_{};  // mutable so const methods can lock it
    mutable std::condition_variable_any state_cv_;
    std::mutex pause_mutex_;
    std::condition_variable pause_cv_;
    
    std::thread thread_;
    std::atomic<bool> running_;
    std::atomic<bool> paused_{ false };
};

} // namespace traffic

#endif // TRAFFIC_SEMAPHORE_HPP
