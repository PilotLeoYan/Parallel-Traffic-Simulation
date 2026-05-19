#pragma once

#include <mutex>
#include <chrono>
#include <cstddef>
#include <functional>
#include <atomic>

namespace city {

struct Coordinate {
    int x;
    int y;

    bool operator==(const Coordinate& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Coordinate& other) const {
        return !(*this == other);
    }

    bool operator<(const Coordinate& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }

    Coordinate operator+(const Coordinate& other) const {
        return Coordinate{x + other.x, y + other.y};
    }

    Coordinate operator-(const Coordinate& other) const {
        return Coordinate{x - other.x, y - other.y};
    }
};

} // namespace city

// Specialize std::hash for city::Coordinate
namespace std {
    template<>
    struct hash<city::Coordinate> {
        size_t operator()(const city::Coordinate& coord) const noexcept {
            size_t h1 = static_cast<size_t>(coord.x);
            size_t h2 = static_cast<size_t>(coord.y);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
} // namespace std

namespace city {

enum class Direction {
    NORTH,
    SOUTH,
    EAST,
    WEST
};

class Intersection {
public:
    Intersection() = default;
    explicit Intersection(size_t id, int x, int y);

    // Non-copyable
    Intersection(const Intersection&) = delete;
    Intersection& operator=(const Intersection&) = delete;

    // Mutex operations (legacy, kept for compatibility)
    void lock();
    bool try_lock();
    void unlock();

    // Status queries
    bool isAvailable() const;
    int getCongestion() const;

    // Accessors
    size_t getId() const;
    Coordinate getCoordinate() const;

private:
    size_t id_;
    int x_;
    int y_;
    mutable std::mutex congestion_mutex_;   // separate lock for congestion reads/writes
    std::atomic<bool> is_occupied_{false};  // single source of truth for occupancy
    int congestion_level_ = 0;
};

}  // namespace city
