#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include "../../src/job-system.h"
#include <chrono>

using namespace Borealis::Jobs;

JobReturnType DoWork(uintptr_t seed = 383628)
{
    // Initialize random number generator
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // Create a vector of random doubles
    std::vector<double> data(300);
    for (auto& elem : data) {
        elem = dis(gen);
    }

    // Perform computationally expensive operations
    for (size_t i = 0; i < 300; ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < 300; ++j) {
            sum += std::sin(data[j]) * std::cos(data[(i + j) % 300]);
        }
        data[i] = std::exp(std::fabs(sum));
    }

    // Sort the vector to add more computation and prevent easy optimization
    std::sort(data.begin(), data.end());
}

int main()
{

    std::cin.get();
    auto start_single_core = std::chrono::steady_clock::now();

    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();
    DoWork();         DoWork();

    auto stop_single_core = std::chrono::steady_clock::now();
    auto interval = stop_single_core - start_single_core;

    printf("Single threaded function calls took %d microseconds!\n", 
        std::chrono::duration_cast<std::chrono::microseconds>(interval).count());

    InitializeJobSystem();

    auto start_multi_core = std::chrono::steady_clock::now();

    Counter jobCounter = Counter(40);
    Job jobs[] =
    {
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
        JOB(&DoWork, &jobCounter, Priority::NORMAL),            JOB(&DoWork, &jobCounter, Priority::NORMAL),
    };

    KickJobs(jobs, 40);

    BusyWaitForCounter(&jobCounter);

    auto stop_multi_core = std::chrono::steady_clock::now();
    auto interval2 = stop_multi_core - start_multi_core;

    printf("Multithreaded function calls took %d microseconds!\n", 
        std::chrono::duration_cast<std::chrono::microseconds>(interval2).count());


    std::cin.get();
    DeinitializeJobSystem();

    std::cin.get();
    return 0;
}