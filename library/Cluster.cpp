#include "Cluster.hpp"

using namespace watchman::simulator;

Clustering::Clustering(std::vector<Router> &tVector) :
    k((int) (sqrtf(sqrtf((float) tVector.size())))) // Optimal cluster size sqrt(sqrt(N)) if only one cluster matches
// and sqrt(2)*sqrt(sqrt(N)) if more than one cluster matches. (In case all routers are distributed equally)
{

    if (tVector.empty()) return;

    // Calculate boundaries
    float minX = tVector[0].position.first, maxX = tVector[0].position.first;
    float minY = tVector[0].position.second, maxY = tVector[0].position.second;

    for (const auto element: tVector) {
        if (element.position.first < minX) minX = element.position.first;
        else if (element.position.first > maxX) maxX = element.position.first;

        if (element.position.second < minY) minY = element.position.second;
        else if (element.position.second > maxY) maxY = element.position.second;
    }

    // Associate routers with clusters
    clusters = std::vector<Cluster>(k * k, Cluster());
    for (auto &element: tVector) {
        int x = (int) floorf((element.position.first - minX) / (maxX - minX + (float)0.00001) * (float) k);
        int y = (int) floorf((element.position.second - minY) / (maxY - minY + (float)0.00001) * (float) k);
        clusters[x + k * y].routers.push_back(&element);
    }

    // Calculate cluster boundaries
    for (auto &cluster: clusters) {
        if (!cluster.routers.empty()) {
            cluster.minX = cluster.routers[0]->position.first - cluster.routers[0]->radius;
            cluster.maxX = cluster.routers[0]->position.first + cluster.routers[0]->radius;
            cluster.minY = cluster.routers[0]->position.second - cluster.routers[0]->radius;
            cluster.maxY = cluster.routers[0]->position.second + cluster.routers[0]->radius;

            for (const auto *element: cluster.routers) {
                if (cluster.minX > element->position.first - element->radius) {
                    cluster.minX = element->position.first - element->radius;
                }
                if (cluster.maxX < element->position.first + element->radius) {
                    cluster.maxX = element->position.first + element->radius;
                }
                if (cluster.minY > element->position.second - element->radius) {
                    cluster.minY = element->position.second - element->radius;
                }
                if (cluster.maxY < element->position.second + element->radius) {
                    cluster.maxY = element->position.second + element->radius;
                }
            }
        } else {
            cluster.minX = cluster.maxX = 0;
            cluster.minY = cluster.maxY = 0;
        }
    }

}

void Clustering::ClusteringIterator::next() {
    if (!_done) {
        if (clusterPos != -1) clusterPos++; // Do not consider current element
        while (true) {
            if (cluster >= clusters.size()) {
                _done = true;
                break;
            }

            if (clusterPos == -1) {
                if (clusters[cluster].minX > object.first || clusters[cluster].maxX < object.first ||
                    clusters[cluster].minY > object.second || clusters[cluster].maxY < object.second) {
                    // Skip cluster
                    cluster++;
                } else {
                    // Enter cluster
                    clusterPos++;
                }
            } else {
                if (clusterPos >= clusters[cluster].routers.size()) {
                    // Leave cluster
                    clusterPos = -1;
                    cluster++;
                } else {
                    if (get().in_reach(object)) {
                        break;
                    } else {
                        clusterPos++;
                    }
                }
            }
        }
    }
}
