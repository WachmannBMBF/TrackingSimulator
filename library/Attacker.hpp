#ifndef CHASE_SIMULATOR_ATTACKER_HPP
#define CHASE_SIMULATOR_ATTACKER_HPP

#include "StreetMap.hpp"

namespace watchman::simulator {
    class Attacker {
    public:
        edge_t edge;
        float fraction;
        position_t position;

        float speed;
        float transmission_prob;
    };
}

#endif //CHASE_SIMULATOR_ATTACKER_HPP
