#include "traffic/semaphore.hpp"

#include <chrono>
#include <thread>
#include <shared_mutex>

namespace traffic {

Semaphore::Semaphore(std::size_t intersection_id,
                     int green_duration,
                     int yellow_duration,
                     int red_duration)
    : intersection_id_(intersection_id)
    , state_(TrafficLightState::RED)  // Start with RED for safety
    , green_duration_(green_duration)
    , yellow_duration_(yellow_duration)
    , red_duration_(red_duration)
    , running_(false)
{
}

Semaphore::~Semaphore() {
    stop();
}

void Semaphore::start() {
    std::unique_lock<std::shared_mutex> lock(rw_mutex_);
    
    if (running_.load()) {
        return;  // Already running
    }
    
    running_.store(true);
    thread_ = std::thread(&Semaphore::cycle, this);
}

void Semaphore::stop() {
    if (!running_.load()) {
        return;  // Not running
    }
    
    running_.store(false);
    state_cv_.notify_all();  // Wake up thread if blocked
    pause_cv_.notify_all();  // Wake up pause condition
    
    if (thread_.joinable()) {
        thread_.join();
    }
}

TrafficLightState Semaphore::getState() const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex_);
    return state_;
}

bool Semaphore::isGreen() const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex_);
    return state_ == TrafficLightState::GREEN || 
           state_ == TrafficLightState::YELLOW;
}

void Semaphore::waitForGreen() const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex_);
    state_cv_.wait(lock, [this]() {
        return state_ == TrafficLightState::GREEN || 
               state_ == TrafficLightState::YELLOW ||
               !running_.load();
    });
}

std::size_t Semaphore::getIntersectionId() const {
    return intersection_id_;
}

void Semaphore::pauseCycle() {
    paused_.store(true);
    state_cv_.notify_all(); // wake up thread if waiting on state_cv_
    pause_cv_.notify_all(); // wake up thread if waiting on pause_cv_
}

void Semaphore::resumeCycle() {
    paused_.store(false);
    pause_cv_.notify_all();
}

void Semaphore::cycle() {
    while (running_.load()) {
        // Green state
        {
            std::unique_lock<std::shared_mutex> lock(rw_mutex_);
            state_ = TrafficLightState::GREEN;
        }
        state_cv_.notify_all();
        
        if (!running_.load()) break;
        std::this_thread::sleep_for(std::chrono::seconds(green_duration_));
        
        // Pause check
        {
            std::unique_lock<std::mutex> lk(pause_mutex_);
            pause_cv_.wait(lk, [this]{ return !paused_.load() || !running_.load(); });
        }
        
        // Yellow state
        {
            std::unique_lock<std::shared_mutex> lock(rw_mutex_);
            state_ = TrafficLightState::YELLOW;
        }
        state_cv_.notify_all();
        
        if (!running_.load()) break;
        std::this_thread::sleep_for(std::chrono::seconds(yellow_duration_));
        
        // Pause check
        {
            std::unique_lock<std::mutex> lk(pause_mutex_);
            pause_cv_.wait(lk, [this]{ return !paused_.load() || !running_.load(); });
        }
        
        // Red state
        {
            std::unique_lock<std::shared_mutex> lock(rw_mutex_);
            state_ = TrafficLightState::RED;
        }
        state_cv_.notify_all();
        
        if (!running_.load()) break;
        std::this_thread::sleep_for(std::chrono::seconds(red_duration_));
        
        // Pause check
        {
            std::unique_lock<std::mutex> lk(pause_mutex_);
            pause_cv_.wait(lk, [this]{ return !paused_.load() || !running_.load(); });
        }
    }
}

} // namespace traffic
