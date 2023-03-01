#ifndef CHASE_SIMULATOR_CLUSTER_HPP
#define CHASE_SIMULATOR_CLUSTER_HPP

#include <utility>

#include "Router.hpp"

namespace watchman::simulator {
    class Cluster {
    public:
        std::vector<Router *> routers;
        float minX = 0, maxX = 0, minY = 0, maxY = 0;
        Cluster() = default;
    };

    class Clustering {
        class ClusteringIterator {
            std::vector<Cluster> &clusters;
            position_t object;
            int cluster = 0;
            int clusterPos = -1;
            bool _done = false;
        public:
            explicit ClusteringIterator(std::vector<Cluster> &clusters, position_t object) : clusters(clusters), object(std::move(object)) {
                next();
            };
            void next();

            Router get() { return *clusters[cluster].routers[clusterPos]; };

            [[nodiscard]] bool hasNext() const { return !_done; };
        };

        std::vector<Cluster> clusters;
        int k;
    public:
        explicit Clustering(std::vector<Router> &tVector);

        ClusteringIterator iterator(position_t object) { return Clustering::ClusteringIterator(clusters, object); };
    };
}

#endif //CHASE_SIMULATOR_CLUSTER_HPP
