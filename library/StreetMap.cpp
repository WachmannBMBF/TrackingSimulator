#include "StreetMap.hpp"
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <algorithm>

using namespace watchman::simulator;

StreetMap::StreetMap() = default;

void StreetMap::add_vertex(int index, position_t position) {
    vertex_positions[index] = position;
}

void StreetMap::add_edge(edge_t e) {
    add_edge(e.first, e.second);
}

void StreetMap::add_edge(int v1, int v2) {
    edges.emplace_back(std::make_pair(v1, v2));
    auto p1 = vertex_positions[v1];
    auto p2 = vertex_positions[v2];
    float dist = sqrtf((p1.first - p2.first) * (p1.first - p2.first) + (p1.second - p2.second) * (p1.second - p2.second));
    weights.push_back(dist);
}

void StreetMap::build_graph() {
    graph = graph_t(vertex_positions.size());
    for (int i = 0; i < edges.size(); i++) {
        auto e = edges[i];
        auto w = weights[i];
        boost::add_edge(e.first, e.second, w, graph);
    }

    property_map<graph_t, vertex_position_t>::type positions = get(vertex_position_t(), graph);
    for (auto pair: vertex_positions) {
        positions[pair.first] = pair.second;
    }
}

position_t StreetMap::get_position(edge_t edge, float fraction) {
    position_t p1 = vertex_positions[edge.first];
    position_t p2 = vertex_positions[edge.second];
    return std::make_pair(p1.first + (p2.first - p1.first) * fraction, p1.second + (p2.second - p1.second) * fraction);
}

float StreetMap::get_edge_length(edge_t edge) {
    std::pair<graph_t::edge_descriptor, bool> ed = boost::edge(edge.first, edge.second, graph);
    return get(edge_weight, graph, ed.first);
}

float StreetMap::shortest_path(std::deque<int> &path, int src, int dest) {
    std::vector<vertex_descriptor> p(num_vertices(graph));
    std::vector<float> d(num_vertices(graph));
    vertex_descriptor source = vertex(src, graph);
    dijkstra_shortest_paths(graph, source,
                            predecessor_map(make_iterator_property_map(p.begin(), get(vertex_index, graph)))
                                .distance_map(make_iterator_property_map(d.begin(), get(vertex_index, graph))));
    vertex_descriptor current = vertex(dest, graph);
    float length = d[current];
    while (current != source) {
        path.push_front((int) current);
        if (current == p[current]) {
            path.clear();
            return 0; // Exit condition if no path exists
        }
        current = p[current];
    }
    return length;
}

float StreetMap::shortest_path(std::deque<int> &path, int v1, int v2, int dest) {
    float length = shortest_path(path, v1, dest);
    if (path[0] != v2) {
        // First node needs to be either v1 or v2
        path.push_front(v1);
    }
    return length;
}

void StreetMap::largest_connected_component(std::vector<int> &largest_cc) const {
    std::vector<int> component(num_vertices(graph));
    int num = connected_components(graph, &component[0]);

    std::vector<int> component_size(num, 0);
    for (auto c: component) {
        component_size[c]++;
    }

    int largest_component = (int) (std::max_element(component_size.begin(), component_size.end()) - component_size.begin());
    for (int v = 0; v < component.size(); v++) {
        if (component[v] == largest_component) {
            largest_cc.push_back(v);
        }
    }
}

void StreetMap::span(const std::vector<int> &vertices, std::vector<std::pair<edge_t, float>> &edges_weights) {
    std::vector<bool> vertex_in_set(num_vertices(graph), false);
    for (auto v: vertices) {
        vertex_in_set[v] = true;
    }

    for (int i = 0; i < edges.size(); i++) {
        auto edge = edges[i];
        if (vertex_in_set[edge.first] && vertex_in_set[edge.second]) {
            edges_weights.emplace_back(std::make_pair(edge, weights[i]));
        }
    }
}

position_t StreetMap::get_position(int vertex) {
    graph_t::vertex_descriptor vd = boost::vertex(vertex, graph);
    return get(vertex_position, graph, vd);
}

float StreetMap::distance(edge_t e1, float f1, edge_t e2, float f2) {
    vertex_descriptor v11 = vertex(e1.first, graph), v12 = vertex(e1.second, graph);
    vertex_descriptor v21 = vertex(e2.first, graph), v22 = vertex(e2.second, graph);
    std::vector<float> *d1, *d2;
    auto it = dijkstra_cache.find(v11);
    if (it == dijkstra_cache.end()) {
        d1 = new std::vector<float>(num_vertices(graph));
        dijkstra_shortest_paths(graph, v11, distance_map(make_iterator_property_map(d1->begin(), get(vertex_index, graph))));
        dijkstra_cache[v11] = d1;
    } else {
        d1 = it->second;
    }
    it = dijkstra_cache.find(v12);
    if (it == dijkstra_cache.end()) {
        d2 = new std::vector<float>(num_vertices(graph));
        dijkstra_shortest_paths(graph, v12, distance_map(make_iterator_property_map(d2->begin(), get(vertex_index, graph))));
        dijkstra_cache[v12] = d2;
    } else {
        d2 = it->second;
    }

    float e1l = get_edge_length(e1), e2l = get_edge_length(e2);
    float p0l = (*d1)[v21] + f1 * e1l + f2 * e2l, p1l = (*d1)[v22] + f1 * e1l + (1 - f2) * e2l,
            p2l = (*d2)[v21] + (1 - f1) * e1l + f2 * e2l, p3l = (*d2)[v22] + (1 - f1) * e1l + (1 - f2) * e2l;
    return std::min(std::min(p0l, p1l), std::min(p2l, p3l));
}

StreetMap::~StreetMap() {
    for (auto e : dijkstra_cache) {
        delete e.second;
    }
}



