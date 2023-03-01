#ifndef CHASE_SIMULATOR_ROUTER_HPP
#define CHASE_SIMULATOR_ROUTER_HPP

#include <utility>
#include "StreetMap.hpp"

namespace watchman::simulator {
        class Router {
        public:
            int id;
            int index;
            edge_t edge;
            float fraction;
            position_t position;

            float radius;
            bool active;
            int activeSince;

            [[nodiscard]] bool in_reach(position_t obj) const;
            [[nodiscard]] bool in_reach(position_t obj, float distance) const;
        };
    }

#endif //CHASE_SIMULATOR_ROUTER_HPP
