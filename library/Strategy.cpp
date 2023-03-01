#include "Strategy.hpp"


#ifdef DEBUG

#include <iostream>

#endif

using namespace boost;
using namespace watchman::simulator;

void Strategy::init(std::vector<Router> &p_routers, std::vector<event_t> &p_events, StreetMap &p_streetMap) {
    hasInit = true;
    routers = &p_routers;
    events = &p_events;
    streetMap = &p_streetMap;
}

void RandomStrategy::setSeed(uint seed) {
    generator.seed(seed);
}

void RandomStrategy::activateRouterFraction(const std::vector<Router *>& routers) {
    std::uniform_real_distribution<float> distribution(0, 1);
#define randreal() (distribution(generator))
    int targetAmount = (int) (routers.size() * fraction);
    int routersLeft = routers.size();
    for (auto router: routers) {
        if (routersLeft > 0 && randreal() < (float) targetAmount / (float) routersLeft) {
            router->active = true;
            router->activeSince = 0;
            targetAmount--;
        }
        routersLeft--;
    }
#undef randreal
}

StaticStrategy::StaticStrategy(float distance) : activationDistance(distance) {}

void StaticStrategy::tick0(const Router &alpha) {
    if (!hasInit) return;

    for (auto &router: *routers) {
        if (router.in_reach(alpha.position, activationDistance)) {
            router.active = true;
            router.activeSince = 0;
        }
    }
}

void StaticStrategy::run() {
    if (!hasInit) return;
}

RadiusStrategy::RadiusStrategy(float distance, int time) :
        activationDistance(distance), maxActivationTime(time) {}

void RadiusStrategy::tick0(const Router &alpha) {
    if (!hasInit) return;

    for (auto &router: *routers) {
        if (router.in_reach(alpha.position, activationDistance)) {
            router.active = true;
            router.activeSince = 0;
        }
    }
}

void RadiusStrategy::run() {
    if (!hasInit) return;

    for (auto &router: *routers) {
        if (router.active && router.activeSince > maxActivationTime) {
            router.active = false;
        }
    }

    bool moveWindow = false;

    for (auto event: *events) {
        if (event.type == router_detects) {
            moveWindow = true;
            break;
        }
    }

    if (moveWindow) {
        for (auto event: *events) {
            if (event.type == router_detects) {
                event.router.activeSince = 0;
                for (auto &router: *routers) {
                    if (router.in_reach(event.router.position, activationDistance)) {
                        router.active = true;
                        router.activeSince = 0;
                    }
                }
            }
        }
    }
}

kSmartestNeighborsStrategy::kSmartestNeighborsStrategy(int k, float maxDist, bool lazy) :
        k(k), maxDist(maxDist), lazy(lazy) {}

bool routerFractionLess(const Router *r1, const Router *r2) {
    return r1->fraction < r2->fraction;
};


void kSmartestNeighborsStrategy::tick0(const Router &alpha) {
    activateRouter(alpha.index);
}

void kSmartestNeighborsStrategy::run() {
    bool moveWindow = false;
    for (auto event: *events) {
        if (event.type == router_detects) {
            moveWindow = true;
            break;
        }
    }

    if (moveWindow) {
        for (auto &router: *routers) {
            router.active = false;
        }

        for (auto event: *events) {
            if (event.type == router_detects) {
                activateRouter(event.router.index);
            }
        }
    }
}

void kSmartestNeighborsStrategy::activateRouter(int index) {
    if (lazy && neighborhoodLists[index].empty()) {
        computeNeighborhoodList(index);
    }

    (*routers)[index].active = true;
    for (auto neighbor: neighborhoodLists[index]) {
        auto &router = (*routers)[neighbor];
        router.active = true;
    }
}

void
kSmartestNeighborsStrategy::init(std::vector<Router> &routers, std::vector<event_t> &events, StreetMap &streetMap) {
    Strategy::init(routers, events, streetMap);
    computeNeighborhoodLists();
}

void kSmartestNeighborsStrategy::computeNeighborhoodLists() {
    neighborhoodLists = std::vector<std::vector<int>>(routers->size());

    // Data structure for efficient access
    for (auto &router: *routers) {
        routersByEdgeSortedByFraction[router.edge].push_back(&router);
    }
    for (auto &edge_router: routersByEdgeSortedByFraction) {
        std::sort(edge_router.second.begin(), edge_router.second.end(), routerFractionLess);
    }

    // Create a copy of the graph with the new edge weights
    copy_graph(streetMap->graph, graph);
    typename graph_traits<graph_t>::edge_iterator ei, eiend;
    for (tie(ei, eiend) = edges(graph); ei != eiend; ++ei) {
        put(edge_weight_t(), graph, *ei, 0);
    }
    for (const auto &edge_routers: routersByEdgeSortedByFraction) {
        auto ed = edge(edge_routers.first.first, edge_routers.first.second, graph);
        put(edge_weight_t(), graph, ed.first, edge_routers.second.size());
    }


    // Multiple Dijkstra should be better than APSP as for large scenarios like villages.osm
    // we have 15000 nodes, but far fewer routers
    std::map<int, std::vector<short>> dijkstra_cache;

    if (!lazy) {
        for (int i = 0; i < routers->size(); i++) {
            computeNeighborhoodList(i, &dijkstra_cache);
        }
    }
}

void kSmartestNeighborsStrategy::computeNeighborhoodList(int i, std::map<int, std::vector<short>> *dijkstra_cache) {
    auto V = num_vertices(graph);
    auto &currentRouter = (*routers)[i];
    auto neighborhood = std::vector<bool>(routers->size(), false);

    // Init first edge
    int kSmall = k, kLarge = k;
    bool foundRouter = false;
    for (const auto &router: routersByEdgeSortedByFraction[currentRouter.edge]) {
        if (router->id == currentRouter.id) {
            foundRouter = true;
        } else if (foundRouter) {
            kLarge--;
            neighborhood[router->index] = true;
            if (!kLarge) break;
        }
    }
    foundRouter = false;
    for (auto it = routersByEdgeSortedByFraction[currentRouter.edge].rbegin();
         it != routersByEdgeSortedByFraction[currentRouter.edge].rend(); it++) {
        if ((*it)->id == currentRouter.id) {
            foundRouter = true;
        } else if (foundRouter) {
            kSmall--;
            neighborhood[(*it)->index] = true;
            if (!kSmall) break;
        }
    }

    // Calculate value for k at the nodes
    std::vector<short> d(V), ds(V), dl(V);
    if (!dijkstra_cache || dijkstra_cache->find(currentRouter.edge.first) == dijkstra_cache->end()) {
        dijkstra_shortest_paths(graph, vertex(currentRouter.edge.first, graph), distance_map(
                make_iterator_property_map(ds.begin(), get(vertex_index, graph))));
        if (dijkstra_cache) (*dijkstra_cache)[currentRouter.edge.first] = ds;
    } else {
        ds = (*dijkstra_cache)[currentRouter.edge.first];
    }
    if (!dijkstra_cache || dijkstra_cache->find(currentRouter.edge.second) == dijkstra_cache->end()) {
        dijkstra_shortest_paths(graph, vertex(currentRouter.edge.second, graph), distance_map(
                make_iterator_property_map(dl.begin(), get(vertex_index, graph))));
        if (dijkstra_cache) (*dijkstra_cache)[currentRouter.edge.second] = dl;
    } else {
        dl = (*dijkstra_cache)[currentRouter.edge.second];
    }
    for (int v = 0; v < V; v++) {
        d[v] = std::max(kSmall - ds[v], kLarge - dl[v]);
    }

    // Compute neighborhood
    for (int v = 0; v < V; v++) {
        if (d[v] > 0) {
            auto s = vertex(v, graph);
            typename graph_traits<graph_t>::adjacency_iterator vi, viend;
            for (boost::tie(vi, viend) = adjacent_vertices(s, graph); vi != viend; ++vi) {
                auto t = *vi;
                int to_activate = d[v];
                edge_t edge = s < t ? edge_t(s, t) : edge_t(t, s);
                bool forward = edge.first == s;
                auto routersOnEdgeIt = routersByEdgeSortedByFraction.find(edge);
                if (routersOnEdgeIt != routersByEdgeSortedByFraction.end()) {
                    auto &vec = routersOnEdgeIt->second;
                    if (forward) {
                        for (auto &router: vec) {
                            if (--to_activate < 0) break;
                            neighborhood[router->index] = true;
                        }
                    } else {
                        for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
                            if (--to_activate < 0) break;
                            neighborhood[(*it)->index] = true;
                        }
                    }
                }
            }
        }
    }

    // Make neighborhood bool vector to int vector
    neighborhoodLists[i] = std::vector<int>(0);
    for (int index = 0; index < neighborhood.size(); index++) {
        if (neighborhood[index] && (*routers)[i].in_reach(currentRouter.position, maxDist))
            neighborhoodLists[i].push_back(index);
    }
}

RandomRadiusStrategy::RandomRadiusStrategy(float distance, int time, float ffraction, uint seed) :
        activationDistance(distance), maxActivationTime(time) {
    setSeed(seed);
    fraction = ffraction;
}

void RandomRadiusStrategy::tick0(const Router &alpha) {
    if (!hasInit) return;

    std::vector<Router*> routersInRange;
    for (auto &router: *routers) {
        if (router.in_reach(alpha.position, activationDistance)) {
            routersInRange.push_back(&router);
        }
    }

    activateRouterFraction(routersInRange);
}

void RandomRadiusStrategy::run() {
    if (!hasInit) return;

    for (auto &router: *routers) {
        if (router.active && router.activeSince > maxActivationTime) {
            router.active = false;
        }
    }

    bool moveWindow = false;

    for (auto event: *events) {
        if (event.type == router_detects) {
            moveWindow = true;
            break;
        }
    }

    if (moveWindow) {
        std::vector<Router*> routersInRange;
        for (auto event: *events) {
            if (event.type == router_detects) {
                event.router.activeSince = 0;
                routersInRange.clear();
                for (auto &router: *routers) {
                    if (router.in_reach(event.router.position, activationDistance)) {
                        routersInRange.push_back(&router);
                    }
                }
                activateRouterFraction(routersInRange);
            }
        }
    }
}

RandomStreetdistanceStrategy::RandomStreetdistanceStrategy(float distance, int time, float ffraction, uint seed) :
        activationDistance(distance), maxActivationTime(time) {
    setSeed(seed);
    fraction = ffraction;
}

void RandomStreetdistanceStrategy::tick0(const Router &alpha) {
    if (!hasInit) return;

    std::vector<Router*> routersInRange;
    for (auto &router: *routers) {
        if (streetMap->distance(alpha.edge, alpha.fraction, router.edge, router.fraction) < activationDistance) {
            routersInRange.push_back(&router);
        }
    }

    activateRouterFraction(routersInRange);
}

void RandomStreetdistanceStrategy::run() {
    if (!hasInit) return;

    for (auto &router: *routers) {
        if (router.active && router.activeSince > maxActivationTime) {
            router.active = false;
        }
    }

    bool moveWindow = false;

    for (auto event: *events) {
        if (event.type == router_detects) {
            moveWindow = true;
            break;
        }
    }

    if (moveWindow) {
        std::vector<Router*> routersInRange;
        for (auto event: *events) {
            if (event.type == router_detects) {
                event.router.activeSince = 0;
                routersInRange.clear();
                for (auto &router: *routers) {
                    if (streetMap->distance(event.router.edge, event.router.fraction, router.edge, router.fraction) < activationDistance) {
                        routersInRange.push_back(&router);
                    }
                }
                activateRouterFraction(routersInRange);
            }
        }
    }
}

void SlidingEuclideanRadiusStrategy::tick0(const Router &alpha) {
    if (!hasInit) return;

    for (auto &router: *routers) {
        router.active = router.in_reach(alpha.position, activationDistance);
    }
}

void SlidingEuclideanRadiusStrategy::run() {
    if (!hasInit) return;

    bool moveWindow = false;

    for (auto event: *events) {
        if (event.type == router_detects) {
            moveWindow = true;
            break;
        }
    }

    if (moveWindow) {
        for (auto &router: *routers) {
            router.active = false;
        }

        for (auto event: *events) {
            if (event.type == router_detects) {
                for (auto &router: *routers) {
                    if (router.in_reach(event.router.position, activationDistance)) {
                        router.active = true;
                    }
                }
            }
        }
    }
}

void SlidingGraphRadiusStrategy::tick0(const Router &alpha) {
    if (!hasInit) return;

    for (auto &router: *routers) {
        router.active = streetMap->distance(alpha.edge, alpha.fraction, router.edge, router.fraction) < activationDistance;
    }
}

void SlidingGraphRadiusStrategy::run() {
    if (!hasInit) return;

    bool moveWindow = false;

    for (auto event: *events) {
        if (event.type == router_detects) {
            moveWindow = true;
            break;
        }
    }

    if (moveWindow) {
        for (auto &router: *routers) {
            router.active = false;
        }

        for (auto event: *events) {
            if (event.type == router_detects) {
                for (auto &router: *routers) {
                    if (streetMap->distance(event.router.edge, event.router.fraction, router.edge, router.fraction) < activationDistance) {
                        router.active = true;
                    }
                }
            }
        }
    }
}
