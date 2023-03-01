#ifndef CHASE_SIMULATOR_SIMULATOR_HPP
#define CHASE_SIMULATOR_SIMULATOR_HPP

#include <iostream>
#include <vector>

#include "Attacker.hpp"
#include "Cluster.hpp"
#include "Router.hpp"
#include "Strategy.hpp"
#include "StreetMap.hpp"
#include "ConcurrentReach.hpp"

namespace watchman::simulator {

    class Simulator {
        StreetMap streetMap;
        std::vector<Router> routers;
        Attacker attacker;
        std::vector<std::pair<edge_t, float>> random_edge_pool;
        std::vector<std::pair<float, float>> cumulative_pool_value;

        Strategy *strategy;
        Clustering *clustering;
        ConcurrentReach *concurrentReach;


        std::pair<bool,int> seed = std::make_pair(false, 0);
        int tick;
        bool done;

        // Precalculations
        const int maxPrecalculations = 8192;
        int precalculationIndex = -1;
        bool precalculationDone = false;
        std::vector<position_t> precalculatedPositions;
        std::vector<std::pair<edge_t, float>> precalculatedGraphPositions;
        std::vector<std::vector<Router>> precalculatedReach;


        // Metrics
        int routerTicks;
        std::map<int, bool> detectionEvents, possibleDetectionEvents;
        int latestDetection, latestPossibleDetection;
        std::vector<edge_t> detectionPoints;

    public:
        struct pathMetric {
            float matching, targetDiff, lengthDiff;
        };

        std::vector<event_t> events;
        std::deque<int> path;
        std::deque<int> completePath;

        explicit Simulator();
        explicit Simulator(int seed);
        void setStrategy(Strategy *p_strategy);
        void addVertex(int vertex, float x, float y);
        void addEdge(int v1, int v2);
        void buildGraph();
        bool setAttacker(int v1, int v2, int target, float fraction, float speed, float tx_prob, int alpha_router_index, float min_path_length);
        [[nodiscard]] Attacker getAttacker() const;
        void addRouter(int id, int v1, int v2, float fraction, float radius);
        Router getRouterByIndex(int index);
        edge_t random_weighted_edge();
        float random_float();
        int random_int(int max);
        [[nodiscard]] int getRouterIndex(int id) const;
        [[nodiscard]] int countRouters() const;
        [[nodiscard]] int getTick() const;
        [[nodiscard]] bool isDone() const;
        void doTick();
        void reset();
        void halfReset();
        void fullReset();

        [[nodiscard]] float metricActivity() const;
        [[nodiscard]] float metricDetection() const;
        [[nodiscard]] float metricLastTracking() const;
        [[nodiscard]] float metricPath();
        [[nodiscard]] struct pathMetric metricPath3();
    };
}

#endif //CHASE_SIMULATOR_SIMULATOR_HPP
