#ifdef DEBUG
#include <iostream>
#endif

#include "ConcurrentReach.hpp"


using namespace watchman::simulator;

ConcurrentReach::ConcurrentReach() :
        clustering(nullptr), threadPool(std::thread::hardware_concurrency()) {
}

std::vector<std::vector<Router>> ConcurrentReach::precalculate(const std::vector<position_t> &positions) {

    size_t numberOfJobs = positions.size() / 32;
    if (numberOfJobs > threadPool.size()) numberOfJobs = threadPool.size();

    std::vector<std::vector<Router>> output(positions.size());
    if (!clustering) return output;
    int done = 0;
    std::mutex doneLock;
    std::condition_variable doneCondition;

    for (int i = 0; i < numberOfJobs; i++) {
        threadPool.addJob([this, &output, numberOfJobs, i, &positions, &done, &doneLock, &doneCondition]() {
            for (size_t j = (i * positions.size()) / numberOfJobs;
                 j < ((i + 1) * positions.size()) / numberOfJobs; j++) {
                for (auto it = clustering->iterator(positions[j]); it.hasNext(); it.next()) {
                    output[j].emplace_back(it.get());
                }
#ifdef DEBUG
                std::cout << "output[" << j << "].size()=" << output[j].size() << std::endl;
#endif
            }

            {
                std::unique_lock<std::mutex> lock(doneLock);
                done += 1;
            }
            doneCondition.notify_one();
        });
    }

    {
        std::unique_lock<std::mutex> lock(doneLock);
        doneCondition.wait(lock, [&done, numberOfJobs]() {
#ifdef DEBUG
//            std::this_thread::sleep_for(std::chrono::milliseconds(50));
//            std::cout << "done=" << done << std::endl;
#endif
            return done == numberOfJobs;
        });
    }
    return output;
}

ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; i++) {
        threads.emplace_back(std::thread([this]() { worker(); }));
    }
}

void ThreadPool::worker() {
    while (true) {
        Job job;
        {
            std::unique_lock<std::mutex> lock(jobsLock);
            jobsCondition.wait(lock, [this]() {
                return !jobs.empty() || done;
            });
            if (done) {
#ifdef DEBUG
                std::cout << "Terminating thread" << std::endl;
#endif
                break;
            }
            job = jobs.front();
            jobs.pop_front();
        }

        job();
    }
}

void ThreadPool::addJob(const ThreadPool::Job &job) {
    {
        std::unique_lock<std::mutex> lock(jobsLock);
        jobs.emplace_back(job);
    }
    jobsCondition.notify_one();
}

ThreadPool::~ThreadPool() {
    done = true;
    jobsCondition.notify_all();
}
