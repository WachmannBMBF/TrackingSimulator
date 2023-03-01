#ifndef CHASE_SIMULATOR_STRATEGY_HPP
#define CHASE_SIMULATOR_STRATEGY_HPP

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/copy.hpp>
#include <random>
#include "Router.hpp"

namespace watchman::simulator {
    typedef enum {
        router_detects,
        router_misses
    } event_type_t;

    typedef struct {
        event_type_t type;
        int tick;
        Router router;
    } event_t;

    class Strategy {
    protected:
        bool hasInit = false;
        std::vector<Router> *routers;
        std::vector<event_t> *events;
        StreetMap *streetMap;
    public:
        virtual ~Strategy() = default;
        virtual void init(std::vector<Router> &routers, std::vector<event_t> &events, StreetMap &streetMap);
        virtual void tick0(const Router &alpha) = 0;
        virtual void run() = 0;
    };

    class RandomStrategy : public Strategy {
    protected:
        float fraction;
        std::default_random_engine generator;
        void setSeed(uint seed);
        void activateRouterFraction(const std::vector<Router*>& routers);
    };

    class StaticStrategy : public Strategy {
        float activationDistance;
    public:
        explicit StaticStrategy(float distance);
        void tick0(const Router &alpha) override;
        void run() override;
    };

    class RadiusStrategy : public Strategy {
        int maxActivationTime;
        float activationDistance;
    public:
        RadiusStrategy(float distance, int time);
        void tick0(const Router &alpha) override;
        void run() override;
    };

    class SlidingEuclideanRadiusStrategy : public Strategy {
        float activationDistance;
    public:
        explicit SlidingEuclideanRadiusStrategy(float distance) : activationDistance(distance) {};
        void tick0(const Router &alpha) override;
        void run() override;
    };

    class SlidingGraphRadiusStrategy : public Strategy {
        float activationDistance;
    public:
        explicit SlidingGraphRadiusStrategy(float distance) : activationDistance(distance) {};
        void tick0(const Router &alpha) override;
        void run() override;
    };

    class kSmartestNeighborsStrategy : public Strategy {
        graph_t graph;
        int k;
        bool lazy;
        float maxDist;
        std::vector<std::vector<int>> neighborhoodLists;
        std::map<edge_t, std::vector<Router *>> routersByEdgeSortedByFraction;

        void activateRouter(int index);
        void computeNeighborhoodLists();
        void computeNeighborhoodList(int index, std::map<int, std::vector<short>> *dijkstra_cache = nullptr);
    public:
        kSmartestNeighborsStrategy(int k, float maxDist, bool lazy = false);
        void init(std::vector<Router> &routers, std::vector<event_t> &events, StreetMap &streetMap) override;
        void tick0(const Router &alpha) override;
        void run() override;
    };

    class RandomRadiusStrategy : public RandomStrategy {
        int maxActivationTime;
        float activationDistance;
    public:
        RandomRadiusStrategy(float distance, int time, float fraction, uint seed);
        void tick0(const Router &alpha) override;
        void run() override;
    };

    class RandomStreetdistanceStrategy : public RandomStrategy {
        int maxActivationTime;
        float activationDistance;
    public:
        RandomStreetdistanceStrategy(float distance, int time, float fraction, uint seed);
        void tick0(const Router &alpha) override;
        void run() override;
    };
}

#endif //CHASE_SIMULATOR_STRATEGY_HPP
