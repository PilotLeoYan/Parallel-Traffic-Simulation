#include "city/street.hpp"
#include "city/intersection.hpp"
#include <stdexcept>
#include <cassert>

namespace city {

StreetSegment::StreetSegment(std::shared_ptr<Intersection> start,
                             std::shared_ptr<Intersection> end,
                             const std::string& name,
                             bool is_two_way)
    : start_intersection_(std::move(start))
    , end_intersection_(std::move(end))
    , name_(name) // <-- NUEVO
    , is_two_way_(is_two_way)
{
    if (!start_intersection_ || !end_intersection_) {
        throw std::invalid_argument("Intersections cannot be null");
    }

    auto start_coord = start_intersection_->getCoordinate();
    auto end_coord = end_intersection_->getCoordinate();

    // Infer direction from coordinates
    if (end_coord.y < start_coord.y) {
        direction_ = Direction::NORTH;
    } else if (end_coord.y > start_coord.y) {
        direction_ = Direction::SOUTH;
    } else if (end_coord.x > start_coord.x) {
        direction_ = Direction::EAST;
    } else if (end_coord.x < start_coord.x) {
        direction_ = Direction::WEST;
    } else {
        throw std::invalid_argument("Start and end intersections must be different");
    }
}

std::shared_ptr<Intersection> StreetSegment::getStartIntersection() const {
    return start_intersection_;
}

std::shared_ptr<Intersection> StreetSegment::getEndIntersection() const {
    return end_intersection_;
}

bool StreetSegment::isTwoWay() const {
    return is_two_way_;
}

std::string StreetSegment::getName() const {
    return name_;
}

Direction StreetSegment::getDirection() const {
    return direction_;
}

bool StreetSegment::canTravel(Direction direction) const {
    if (direction == direction_) {
        return true;
    }
    if (is_two_way_ && direction == getOppositeDirection()) {
        return true;
    }
    return false;
}

Direction StreetSegment::getOppositeDirection() const {
    switch (direction_) {
        case Direction::NORTH: return Direction::SOUTH;
        case Direction::SOUTH: return Direction::NORTH;
        case Direction::EAST: return Direction::WEST;
        case Direction::WEST: return Direction::EAST;
    }
    return direction_;
}

}  // namespace city
