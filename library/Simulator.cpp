#include <random>
#include <thread>

#include "Simulator.hpp"

using namespace watchman::simulator;

static std::default_random_engine generator;
static std::uniform_real_distribution<float> float_distribution(0, 1);

Simulator::Simulator() : Simulator(0) {
    // And we directly re-seed the generator here
    static std::random_device rd{};
    generator.seed(rd());
}

Simulator::Simulator(const int seed) : tick(0), done(false), strategy(nullptr), clustering(nullptr), detectionEvents(),
                                       concurrentReach(new ConcurrentReach()),
                                       possibleDetectionEvents(), routerTicks(0), latestDetection(0), latestPossibleDetection(0) {
    generator.seed(seed);
    this->seed = std::make_pair(true, seed);
}

#ifdef DEBUG

void printPath(std::deque<int> &path, int max = 5) {
    std::cout << "path";
    for (int p: path) {
        if (max-- == 0) break;
        std::cout << " - " << p;
    }
    std::cout << std::endl;
}

#endif

void Simulator::doTick() {
    if (done) return;
    tick++;
    events.clear();

    if (precalculationIndex < 0 || precalculationIndex >= precalculatedPositions.size()) {
        if (precalculationDone) {
            done = true;
            return;
        }

        precalculationIndex = 0;
#ifdef DEBUG
        std::cout << "Clearing positions" << std::endl;
#endif
        precalculatedPositions.clear();
        precalculatedGraphPositions.clear();
        precalculatedReach.clear();

        for (int i = 0; i < maxPrecalculations; i++) {
            // Move attacker
            float distance = attacker.speed;
            while (distance > 0 && !precalculationDone) {
                float edge_length = streetMap.get_edge_length(attacker.edge);
                bool walk_forwards = path.front() == attacker.edge.second;
                float remaining_fraction = walk_forwards ? 1 - attacker.fraction : attacker.fraction;

                if (remaining_fraction * edge_length > distance) {
                    // We stay on this edge
                    attacker.fraction = walk_forwards ? attacker.fraction + distance / edge_length : attacker.fraction -
                                                                                                     distance /
                                                                                                     edge_length;
                    distance = 0;
                } else {
                    // We move to the next edge
                    distance -= remaining_fraction * edge_length;
                    int curr = path.front();
                    path.pop_front();
                    if (path.empty()) {
                        // We are done
                        precalculationDone = true;
                        attacker.fraction = 1;
                        break;
                    }
                    int next = path.front();
                    if (curr < next) {
                        attacker.edge = std::make_pair(curr, next);
                        attacker.fraction = 0;
                    } else {
                        attacker.edge = std::make_pair(next, curr);
                        attacker.fraction = 1;
                    }
                }
            }
            attacker.position = streetMap.get_position(attacker.edge, attacker.fraction);
            precalculatedPositions.push_back(attacker.position);
            precalculatedGraphPositions.emplace_back(std::make_pair(attacker.edge, attacker.fraction));
            if (precalculationDone) break;
        }
        precalculatedReach = concurrentReach->precalculate(precalculatedPositions);
    }

    attacker.edge = precalculatedGraphPositions[precalculationIndex].first;
    attacker.fraction = precalculatedGraphPositions[precalculationIndex].second;
    attacker.position = precalculatedPositions[precalculationIndex];

#ifdef DEBUG
    //    std::cout << "tick=" << tick << " edge=(" << attacker.edge.first << "," << attacker.edge.second << ") fraction="
    //              << attacker.fraction << " pos=(" << attacker.position.first << "," << attacker.position.second << ")"
    //              << std::endl;
#endif

    for (auto router: routers) {
        if (router.active) {
#ifdef DEBUG
            std::cout << "Router active id=" << router.index << std::endl;
#endif
            routerTicks++;
        }
    }

    float r = random_float();
    if (attacker.transmission_prob > r) {
        // The attacker sends a packet
        edge_t *detection = nullptr;
        for (auto &router: precalculatedReach[precalculationIndex]) { // Note router in precalculatedReach is not synced with routers
            if (routers[router.index].active) {
                events.push_back({router_detects, tick, routers[router.index]});
                detectionEvents[router.index] = true;
                latestDetection = tick;
                // todo router.edge should model the scenario better, but complicates the path reconstruction
                detection = &attacker.edge;
            } else {
                events.push_back({router_misses, tick, routers[router.index]});
            }
            possibleDetectionEvents[router.index] = true;
            latestPossibleDetection = tick;
        }
        if (detection) {
            detectionPoints.push_back(*detection);
        }
    } else {
        // The attacker does not send a packet
        for (auto router: precalculatedReach[precalculationIndex]) {
            possibleDetectionEvents[router.index] = true;
            latestPossibleDetection = tick;
        }
    }
    precalculationIndex++;

    for (auto &router: routers) { // todo move to strategy
        if (router.active) {
            router.activeSince++;
        }
    }
    if (strategy) {
        strategy->run();
    }
}

void Simulator::reset() {
    events.clear();

    routerTicks = 0;
    detectionEvents.clear();
    possibleDetectionEvents.clear();
    latestDetection = 0;
    latestPossibleDetection = 0;
    detectionPoints.clear();

    precalculationDone = false;
    precalculationIndex = -1;
    precalculatedPositions.clear();
#ifdef DEBUG
    std::cout << "Clearing positions" << std::endl;
#endif
    precalculatedGraphPositions.clear();
    precalculatedReach.clear();

    done = false;
    tick = 0;
}

void Simulator::halfReset() {
    this->reset();
    delete strategy;
    strategy = nullptr;
    if (seed.first) generator.seed(seed.second);
    routers.clear();
}

void Simulator::fullReset() {
    this->halfReset();
    streetMap = StreetMap();
}

void Simulator::setStrategy(Strategy *p_strategy) {
    delete strategy;
    delete clustering;
    strategy = p_strategy;
    strategy->init(routers, events, streetMap);
    clustering = new Clustering(routers);
    concurrentReach->setClustering(clustering);
}

int Simulator::getTick() const {
    return tick;
}

bool Simulator::isDone() const {
    return done;
}

void Simulator::addVertex(int vertex, float x, float y) {
    streetMap.add_vertex(vertex, std::make_pair(x, y));
}

void Simulator::addEdge(int v1, int v2) {
    if (v1 > v2) std::swap(v1, v2);
    streetMap.add_edge(v1, v2);
}

void Simulator::buildGraph() {
    streetMap.build_graph();
}

bool
Simulator::setAttacker(int v1, int v2, int target, float fraction, float speed, float tx_prob, int alpha_router_index,
                       float min_path_length) {
    attacker.edge = std::make_pair(v1, v2);
    attacker.fraction = fraction;
    attacker.position = streetMap.get_position(attacker.edge, attacker.fraction);
    attacker.speed = speed;
    attacker.transmission_prob = tx_prob;

    path.clear();
    float path_length = streetMap.shortest_path(path, v1, v2, target);
    completePath = std::deque<int>(path);

    for (auto &router: routers) {
        router.active = false;
        router.activeSince = 0;
    }

    detectionPoints.push_back(attacker.edge);
    if (strategy) {
        strategy->tick0(routers[alpha_router_index]);
    }
#ifdef DEBUG
    else {
        std::cout << "Strategy is not set" << std::endl;
    }
#endif
    // std::cout << "path_length=" << path_length << ", min_path_length=" << min_path_length << std::endl;
    return path_length >= min_path_length;
}

void Simulator::addRouter(int id, int v1, int v2, float fraction, float radius) {
    Router router;
    router.id = id;
    router.index = (int) routers.size();
    router.edge = v1 < v2 ? edge_t(v1, v2) : edge_t(v2, v1);
    router.fraction = fraction;
    router.position = streetMap.get_position(router.edge, router.fraction);
    router.radius = radius;
    routers.emplace_back(router);
}

Router Simulator::getRouterByIndex(int index) {
    return routers[index];
}

int Simulator::countRouters() const {
    return (int) routers.size();
}

Attacker Simulator::getAttacker() const {
    return attacker;
}

int Simulator::getRouterIndex(int id) const {
    if (id < routers.size() && routers[id].id == id) {
        // Heuristic
        return id;
    }
    for (int i = 0; i < routers.size(); i++) {
        if (routers[i].id == id) {
            return i;
        }
    }
    return -1;
}

edge_t Simulator::random_weighted_edge() {
    if (random_edge_pool.empty()) {
        std::vector<int> largest_cc;
        streetMap.largest_connected_component(largest_cc);
        streetMap.span(largest_cc, random_edge_pool);
        float current = 0;
        for (auto e_w: random_edge_pool) {
            cumulative_pool_value.emplace_back(std::make_pair(current, current + e_w.second));
            current += e_w.second;
        }
    }

    float pool_size = cumulative_pool_value.back().second;
    int min = 0, max = (int) random_edge_pool.size(); // We are currently searching in [min,max)
    float rand_point = random_float() * pool_size;
    while (min < max - 1) {
        auto considered = min + (max - min) / 2;
        if (cumulative_pool_value[considered].first > rand_point) {
            max = considered;
        } else if (cumulative_pool_value[considered].second < rand_point) {
            min = considered + 1;
        } else {
            return random_edge_pool[considered].first;
        }
    }

    return random_edge_pool[min].first;
}

float Simulator::metricActivity() const {
    return 1 - (float) routerTicks / (float) (tick * routers.size());
}

float Simulator::metricDetection() const {
    if (possibleDetectionEvents.empty()) return 0;
    return (float) detectionEvents.size() / (float) possibleDetectionEvents.size();
}

float Simulator::metricLastTracking() const {
    return (float) latestDetection / (float) latestPossibleDetection;
}

float Simulator::metricPath() {
    auto path3 = metricPath3();
    return (path3.matching + 2 * path3.lengthDiff * path3.targetDiff) / 3;
}

float Simulator::random_float() {
    return float_distribution(generator);
}

int Simulator::random_int(int max) {
    int random = (std::uniform_int_distribution<int>(0, max))(generator);
    return random;
}

Simulator::pathMetric Simulator::metricPath3() {
    // Reconstruct path
    std::vector<int> reconstructedPath;
    std::deque<int> pathSegment;
    for (int i = 1; i < detectionPoints.size(); i++) {
        pathSegment.clear();
        edge_t d1 = detectionPoints[i - 1], d2 = detectionPoints[i];
        streetMap.shortest_path(pathSegment, d1.first,
                                d2.first); // Depending on the number of detection points APSP might be better

        if (pathSegment.empty()) continue; // this should normally not happen, but we want to prevent the function from crashing
        if (reconstructedPath.empty()) {
            if (pathSegment.front() != d1.second) // First node should be either d1.first or d1.second
                reconstructedPath.push_back(d1.first);
        }
        for (auto v: pathSegment) {
            if (reconstructedPath.empty() || reconstructedPath.back() != v && reconstructedPath.back() != v)
                reconstructedPath.push_back(v);
        }
        bool push_other_node = false;
        if (pathSegment.size() > 1) {
            int second_to_last = *(pathSegment.end() - 1);
            if (second_to_last != d2.first && second_to_last != d2.second) {
                push_other_node = true;
            }
        }
        if (push_other_node) {
            reconstructedPath.push_back(pathSegment.back() == d2.first ? d2.second : d2.first);
        }
    }

    std::map<edge_t, bool> edges, reconstructedEdges;
    for (int i = 1; i < completePath.size(); i++) {
        auto v1 = completePath[i - 1], v2 = completePath[i];
        if (v1 < v2)
            edges[std::make_pair(v1, v2)] = true;
        else
            edges[std::make_pair(v2, v1)] = true;
    }
    for (int i = 1; i < reconstructedPath.size(); i++) {
        auto v1 = reconstructedPath[i - 1], v2 = reconstructedPath[i];
        if (v1 < v2)
            reconstructedEdges[std::make_pair(v1, v2)] = true;
        else
            reconstructedEdges[std::make_pair(v2, v1)] = true;
    }
    float pathLength = 0, reconstructedLength = 0, intersectingLength = 0;
    for (auto edge_exists: edges) {
        pathLength += streetMap.get_edge_length(edge_exists.first);
    }
    for (auto edge_exists: reconstructedEdges) {
        float edge_length = streetMap.get_edge_length(edge_exists.first);
        reconstructedLength += edge_length;
        if (edges.find(edge_exists.first) != edges.end()) {
            intersectingLength += edge_length;
        }
    }

    if (pathLength == 0 || reconstructedLength == 0) return {0, 0, 0};

    auto t1 = streetMap.get_position(completePath.back()), t2 = streetMap.get_position(reconstructedPath.back());
    float diff_target = sqrtf(
        (t1.first - t2.first) * (t1.first - t2.first) + (t1.second - t2.second) * (t1.second - t2.second));
    float diff_length = pathLength - reconstructedLength;
    if (diff_length < 0) diff_length = -diff_length;
    float matching = intersectingLength / pathLength,
        lengthDiff = 1 / (1 + diff_length),
        targetDiff = 1 / (1 + diff_target);


    return {matching, targetDiff, lengthDiff};
}
