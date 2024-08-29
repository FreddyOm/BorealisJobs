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
    std::vector<double> data(900);
    for (auto& elem : data) {
        elem = dis(gen);
    }

    // Perform computationally expensive operations
    for (size_t i = 0; i < 900; ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < 900; ++j) {
            sum += std::sin(data[j]) * std::cos(data[(i + j) % 900]);
        }
        data[i] = std::exp(std::fabs(sum));
    }

    // Sort the vector to add more computation and prevent easy optimization
    std::sort(data.begin(), data.end());
}

int main()
{
    printf("Press ENTER to start the heavy work on a single core... \n");
    std::cin.get();
    printf("Doing work... \n");

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

    printf("The work was executed in one single thread in %d milliseconds!\n", 
        std::chrono::duration_cast<std::chrono::milliseconds>(interval).count());

    printf("Press ENTER to start the heavy work on %d cores... \n", (std::thread::hardware_concurrency() - 1));
    std::cin.get();
    printf("Doing work... \n");

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

    WaitForCounter(&jobCounter);

    auto stop_multi_core = std::chrono::steady_clock::now();
    auto interval2 = stop_multi_core - start_multi_core;

    printf("The work was executed in %d threads in %d milliseconds!\nPress ENTER to close... ", std::thread::hardware_concurrency() - 1,
        std::chrono::duration_cast<std::chrono::milliseconds>(interval2).count());

    std::cin.get();

    DeinitializeJobSystem();

    return 0;
}