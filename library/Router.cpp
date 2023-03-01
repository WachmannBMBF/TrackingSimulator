#include "Router.hpp"

using namespace watchman::simulator;

static inline float norm2(float x, float y) {
    return x * x + y * y;
}

bool Router::in_reach(position_t object) const {
    return in_reach(object, radius);
}

bool Router::in_reach(position_t object, float distance) const {
    return norm2(position.first - object.first, position.second - object.second) <= distance * distance;
}