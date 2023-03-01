#ifndef CHASE_SIMULATOR_STREETMAP_HPP
#define CHASE_SIMULATOR_STREETMAP_HPP

#include <utility>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

using namespace boost;

namespace boost {
    enum vertex_position_t {
        vertex_position
    };
    template<>
    struct property_kind<vertex_position_t> {
        typedef vertex_property_tag type;
    };
}

namespace watchman::simulator {

    typedef std::pair<float, float> position_t;
    typedef std::pair<int, int> edge_t;

    typedef adjacency_list<vecS, vecS, undirectedS,
        property<vertex_position_t, position_t, property<vertex_rank_t, int>>,
        property<edge_weight_t, float>> graph_t;

    class StreetMap {
        typedef graph_traits< graph_t >::vertex_descriptor vertex_descriptor;
        std::map<int, position_t> vertex_positions;
        std::vector<edge_t> edges;
        std::vector<float> weights;
        std::map<vertex_descriptor, std::vector<float>*> dijkstra_cache;
    public:
        graph_t graph;
        StreetMap();
        ~StreetMap();
        void add_vertex(int index, position_t position);
        void add_edge(edge_t e);
        void add_edge(int v1, int v2);
        void build_graph();
        position_t get_position(int vertex);
        position_t get_position(edge_t edge, float fraction);
        float get_edge_length(edge_t edge);

        void largest_connected_component(std::vector<int> &largest_cc) const;
        void span(const std::vector<int> &vertices, std::vector<std::pair<edge_t, float>> &edges_weights);

        float shortest_path(std::deque<int> &path, int src, int dest);
        float shortest_path(std::deque<int> &path, int v1, int v2, int dest);

        float distance(edge_t e1, float f1, edge_t e2, float f2);
    };
}

#endif //CHASE_SIMULATOR_STREETMAP_HPP
