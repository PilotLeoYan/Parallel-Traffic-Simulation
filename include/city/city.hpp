#pragma once

#include "intersection.hpp"
#include <vector>
#include <memory>

namespace city {

class StreetSegment;

class City {
public:
    /**
     * @brief Get the singleton instance of City
     * @return Reference to the singleton City instance
     */
    static City& getInstance() {
        static City instance;
        return instance;
    }

    City() = default;

    // Non-copyable, non-movable
    City(const City&) = delete;
    City& operator=(const City&) = delete;
    City(City&&) = delete;
    City& operator=(City&&) = delete;

    // Initialize the city grid
    void initialize(int grid_size);

    // Intersection access
    std::shared_ptr<Intersection> getIntersection(const Coordinate& coord) const;
    std::shared_ptr<Intersection> getIntersectionAt(int x, int y) const;

    // Grid queries
    std::vector<std::shared_ptr<Intersection>> getNeighbors(const Coordinate& coord) const;
    bool isValidCoordinate(const Coordinate& coord) const;

    // Street management
    void addStreet(std::shared_ptr<StreetSegment> street);
    const std::vector<std::shared_ptr<StreetSegment>>& getStreets() const;

    std::shared_ptr<StreetSegment> getStreet(const Coordinate& from, const Coordinate& to) const;

    // Grid properties
    int getGridSize() const;

    // For testing: check if grid is properly initialized
    bool isInitialized() const;

private:
    int grid_size_ = 0;
    std::vector<std::vector<std::shared_ptr<Intersection>>> intersections_;
    std::vector<std::shared_ptr<StreetSegment>> streets_;

    void initializeIntersections();
    void initializeStreets();
};

}  // namespace city
