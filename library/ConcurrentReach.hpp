#ifndef CHASE_SIMULATOR_CONCURRENTREACH_H
#define CHASE_SIMULATOR_CONCURRENTREACH_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>
#include "Cluster.hpp"

namespace watchman::simulator {

    class ThreadPool {
        typedef std::function<void()> Job;

        std::vector<std::thread> threads;
        std::deque<Job> jobs;
        std::mutex jobsLock;
        std::condition_variable jobsCondition;

        bool done = false;

        void worker();

    public:
        explicit ThreadPool(size_t num_threads);
        ~ThreadPool();

        void addJob(const Job &job);

        [[nodiscard]] auto size() const { return threads.size(); };
    };

    class ConcurrentReach {
        Clustering *clustering;
        ThreadPool threadPool;
    public:
        explicit ConcurrentReach();

        std::vector<std::vector<Router>> precalculate(const std::vector<position_t> &positions);

        void setClustering(Clustering *pClustering) { clustering = pClustering; };
    };

}

#endif //CHASE_SIMULATOR_CONCURRENTREACH_H
