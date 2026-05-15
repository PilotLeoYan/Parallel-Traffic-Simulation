#pragma once

#include "intersection.hpp"
#include <memory>
#include <string>

namespace city {

class Intersection;

class StreetSegment {
public:
    StreetSegment() = default;
    StreetSegment(std::shared_ptr<Intersection> start,
                  std::shared_ptr<Intersection> end,
                  const std::string& name,
                  bool is_two_way = true);

    // Non-copyable
    StreetSegment(const StreetSegment&) = delete;
    StreetSegment& operator=(const StreetSegment&) = delete;

    // Accessors
    std::shared_ptr<Intersection> getStartIntersection() const;
    std::shared_ptr<Intersection> getEndIntersection() const;
    bool isTwoWay() const;
    Direction getDirection() const;
    
    // NUEVO: Getter para el nombre
    std::string getName() const;

    bool canTravel(Direction direction) const;
    Direction getOppositeDirection() const;

private:
    std::shared_ptr<Intersection> start_intersection_;
    std::shared_ptr<Intersection> end_intersection_;
    std::string name_; // <-- NUEVO
    bool is_two_way_;
    Direction direction_;
};

}  // namespace city
