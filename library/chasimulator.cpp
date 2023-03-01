#include <iostream>

#ifdef EMSCRIPTEN

#include <emscripten.h>

#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "Simulator.hpp"

using namespace watchman::simulator;

static Simulator globalSimulator = Simulator();
static event_t event;
static edge_t random_edge;

extern "C" {
EMSCRIPTEN_KEEPALIVE void add_vertex(int vertex, float x, float y) {
    globalSimulator.addVertex(vertex, x, y);
}

EMSCRIPTEN_KEEPALIVE void add_edge(int v1, int v2) {
    globalSimulator.addEdge(v1, v2);
}

EMSCRIPTEN_KEEPALIVE void build_graph() {
    globalSimulator.buildGraph();
}

EMSCRIPTEN_KEEPALIVE void set_sample_strategy(float distance) {
    auto sampleStrategy = new StaticStrategy(distance);
    globalSimulator.setStrategy(sampleStrategy);
}

EMSCRIPTEN_KEEPALIVE void set_sliding_window_strategy(float distance, int time) {
    auto slidingWindowStrategy = new RadiusStrategy(distance, time);
    globalSimulator.setStrategy(slidingWindowStrategy);
}

EMSCRIPTEN_KEEPALIVE void set_k_smartest_neighbors_strategy(int k, float maxDist, bool lazy) {
    auto strategy = new kSmartestNeighborsStrategy(k, maxDist, lazy);
    globalSimulator.setStrategy(strategy);
}

EMSCRIPTEN_KEEPALIVE
bool set_attacker(int v1, int v2, int target, float fraction, float speed, float tx_prob, int alpha_router_index, float min_path_length) {
    return globalSimulator.setAttacker(v1, v2, target, fraction, speed, tx_prob, alpha_router_index, min_path_length);
}

EMSCRIPTEN_KEEPALIVE int get_attacker_v1() {
    return globalSimulator.getAttacker().edge.first;
}

EMSCRIPTEN_KEEPALIVE int get_attacker_v2() {
    return globalSimulator.getAttacker().edge.second;
}

EMSCRIPTEN_KEEPALIVE float get_attacker_fraction() {
    return globalSimulator.getAttacker().fraction;
}

EMSCRIPTEN_KEEPALIVE float get_attacker_x() {
    return globalSimulator.getAttacker().position.first;
}

EMSCRIPTEN_KEEPALIVE float get_attacker_y() {
    return globalSimulator.getAttacker().position.second;
}

EMSCRIPTEN_KEEPALIVE float get_attacker_speed() {
    return globalSimulator.getAttacker().speed;
}

EMSCRIPTEN_KEEPALIVE float get_attacker_tx_prob() {
    return globalSimulator.getAttacker().transmission_prob;
}

EMSCRIPTEN_KEEPALIVE void add_router(int id, int v1, int v2, float fraction, float radius) {
    globalSimulator.addRouter(id, v1, v2, fraction, radius);
}

EMSCRIPTEN_KEEPALIVE int get_router_id_by_index(int index) {
    return globalSimulator.getRouterByIndex(index).id;
}

EMSCRIPTEN_KEEPALIVE int get_router_v1_by_index(int index) {
    return globalSimulator.getRouterByIndex(index).edge.first;
}

EMSCRIPTEN_KEEPALIVE int get_router_v2_by_index(int index) {
    return globalSimulator.getRouterByIndex(index).edge.second;
}

EMSCRIPTEN_KEEPALIVE float get_router_fraction_by_index(int index) {
    return globalSimulator.getRouterByIndex(index).fraction;
}

EMSCRIPTEN_KEEPALIVE float get_router_x_by_index(int index) {
    return globalSimulator.getRouterByIndex(index).position.first;
}

EMSCRIPTEN_KEEPALIVE float get_router_y_by_index(int index) {
    return globalSimulator.getRouterByIndex(index).position.second;
}

EMSCRIPTEN_KEEPALIVE float get_router_radius_by_index(int index) {
    return globalSimulator.getRouterByIndex(index).radius;
}

EMSCRIPTEN_KEEPALIVE int get_router_active_by_index(int index) {
    return (int) globalSimulator.getRouterByIndex(index).active;
}

EMSCRIPTEN_KEEPALIVE int get_router_active_since_by_index(int index) {
    return globalSimulator.getRouterByIndex(index).activeSince;
}

EMSCRIPTEN_KEEPALIVE int get_router_index_by_id(int id) {
    return globalSimulator.getRouterIndex(id);
}

EMSCRIPTEN_KEEPALIVE float metric_activity() {
    return globalSimulator.metricActivity();
}

EMSCRIPTEN_KEEPALIVE float metric_detection() {
    return globalSimulator.metricDetection();
}

EMSCRIPTEN_KEEPALIVE float metric_last_tracking() {
    return globalSimulator.metricLastTracking();
}

EMSCRIPTEN_KEEPALIVE float metric_path() {
    return globalSimulator.metricPath();
}

EMSCRIPTEN_KEEPALIVE int count_routers() {
    return globalSimulator.countRouters();
}

EMSCRIPTEN_KEEPALIVE int get_path_length() {
    return (int) globalSimulator.path.size();
}

EMSCRIPTEN_KEEPALIVE int get_path_node(int index) {
    return (int) globalSimulator.path[index];
}

EMSCRIPTEN_KEEPALIVE bool pop_event() {
    if (globalSimulator.events.empty()) return false;

    event = globalSimulator.events.back();
    globalSimulator.events.pop_back();
    return true;
}

EMSCRIPTEN_KEEPALIVE bool event_is_router_detects() {
    return event.type == watchman::simulator::router_detects;
}

EMSCRIPTEN_KEEPALIVE bool event_is_router_misses() {
    return event.type == watchman::simulator::router_misses;
}

EMSCRIPTEN_KEEPALIVE int event_router_get_index() {
    return globalSimulator.getRouterIndex(event.router.id);
}

EMSCRIPTEN_KEEPALIVE int event_get_tick() {
    return event.tick;
}

EMSCRIPTEN_KEEPALIVE void random_weighted_edge_new() {
    random_edge = globalSimulator.random_weighted_edge();
}

EMSCRIPTEN_KEEPALIVE int random_weighted_edge_v1() {
    return random_edge.first;
}

EMSCRIPTEN_KEEPALIVE int random_weighted_edge_v2() {
    return random_edge.second;
}

EMSCRIPTEN_KEEPALIVE void do_tick() {
    globalSimulator.doTick();
}

EMSCRIPTEN_KEEPALIVE int get_tick() {
    return globalSimulator.getTick();
}

EMSCRIPTEN_KEEPALIVE bool is_done() {
    return globalSimulator.isDone();
}

EMSCRIPTEN_KEEPALIVE void reset() {
    globalSimulator.reset();
}

EMSCRIPTEN_KEEPALIVE void full_reset() {
    globalSimulator.fullReset();
}
}

#ifndef EMSCRIPTEN
int main() {
    using namespace std;

    full_reset();
    add_vertex(0, 0, 0);
    add_vertex(1, 1, 0);
    add_vertex(2, 2, 0);
    add_vertex(3, 3, 0);
    add_vertex(4, 4, 0);
    add_vertex(5, 2, 2);
    add_edge(0, 1);
    add_edge(1, 2);
    add_edge(2, 3);
    add_edge(3, 4);
    add_edge(4, 5);
    add_edge(0, 5);
    add_router(0, 0, 1, 0.5, 2);
    build_graph();
    globalSimulator.setStrategy(new SlidingGraphRadiusStrategy(1));
//    cout << globalSimulator.streetMap.distance(make_pair(0, 1), 0.5, make_pair(0, 5), 1) << endl;
//    cout << globalSimulator.streetMap.distance(make_pair(0, 5), 1, make_pair(0, 1), 0.5) << endl;
}
#endif // not def EMSCRIPTEN
