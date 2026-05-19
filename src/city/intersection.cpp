#include "city/intersection.hpp"
#include <stdexcept>
#include <thread>

namespace city {

Intersection::Intersection(size_t id, int x, int y)
    : id_(id), x_(x), y_(y), congestion_level_(0) {
    is_occupied_.store(false); }

void Intersection::lock() {
    // Spin until we can claim the atomic
    while (!try_lock()) {
        std::this_thread::yield();
    }
}

bool Intersection::try_lock() {
    // Atomic claim: exchange is_occupied_ from false -> true in one CAS step.
    // Returns true only if WE made the transition.
    bool expected = false;
    return is_occupied_.compare_exchange_strong(expected, true,
                                                std::memory_order_acquire,
                                                std::memory_order_relaxed);
}

void Intersection::unlock() {
    is_occupied_.store(false, std::memory_order_release);
}

bool Intersection::isAvailable() const {
    return !is_occupied_.load(std::memory_order_acquire);
}

int Intersection::getCongestion() const {
    std::lock_guard<std::mutex> lock(congestion_mutex_);
    return congestion_level_;
}

size_t Intersection::getId() const {
    return id_;
}

Coordinate Intersection::getCoordinate() const {
    return {x_, y_};
}

}  // namespace city
