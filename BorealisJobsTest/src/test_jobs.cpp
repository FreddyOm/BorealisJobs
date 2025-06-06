#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

#include <gtest/gtest.h>
#include "../../src/job-system.h"

#ifdef BOREALIS_WIN

using namespace Borealis::Jobs;

// Object related test functions
struct TestStruct
{
    JobReturnType DoTestWork()
    {
        std::mt19937 gen(98349109847);
        std::uniform_int_distribution<> dis(0, INT_MAX);

        static const int maxNum = dis(gen);

        while (testInt2 < maxNum)
        {
            const int temp1 = testInt1;
            const int temp2 = testInt2;

            testInt2 = temp1 + temp2;
            testInt1 = temp2;
        }
    }

    JobReturnType DoTestWorkWithArgs(unsigned long long _maxNum)
    {
        static const int maxNum = _maxNum;

        while (testInt2 < maxNum)
        {
            const int temp1 = testInt1;
            const int temp2 = testInt2;

            testInt2 = temp1 + temp2;
            testInt1 = temp2;
        }
    }

    int testInt1 = 1;
    int testInt2 = 2;
};

// Global test function
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


TEST(BorealisJobsTest, TestGeneralUsage)
{
    InitializeJobSystem();

    TestStruct myTestStruct;

    EXPECT_EQ(myTestStruct.testInt1, 1);
    EXPECT_EQ(myTestStruct.testInt2, 2);

    Counter jobCounter = Counter(2);
    EXPECT_EQ(jobCounter, 2);

    KickJob(JOB(BIND(TestStruct::DoTestWork, myTestStruct), &jobCounter, Priority::HIGH));
    KickJob(JOB(&DoWork, &jobCounter, Priority::HIGH));
    WaitForCounter(&jobCounter);

    EXPECT_GT(myTestStruct.testInt1, 1);
    EXPECT_GT(myTestStruct.testInt2, 2);
    EXPECT_EQ(jobCounter, 0);

    DeinitializeJobSystem();
}

TEST(BorealisJobsTest, TestArguments)
{
    InitializeJobSystem();

    TestStruct myTestStruct;
    EXPECT_EQ(myTestStruct.testInt1, 1);
    EXPECT_EQ(myTestStruct.testInt2, 2);

    Counter jobCounter = Counter(2);
    EXPECT_EQ(jobCounter, 2);

    KickJob(JOB(BIND(TestStruct::DoTestWorkWithArgs, myTestStruct, 9999999999), &jobCounter, Priority::HIGH));
    KickJob(JOB(&DoWork, &jobCounter, Priority::HIGH, 9999999999));
    WaitForCounter(&jobCounter);

    EXPECT_GT(myTestStruct.testInt1, 1);
    EXPECT_GT(myTestStruct.testInt2, 2);

    EXPECT_LT(myTestStruct.testInt1, 9999999999);
    EXPECT_LT(myTestStruct.testInt2, 9999999999);
    
    EXPECT_EQ(jobCounter, 0);


    DeinitializeJobSystem();
}

TEST(BorealisJobsTest, TestBatchJobs)
{
    InitializeJobSystem();

    Counter jobCounter = Counter(40);
    EXPECT_EQ(jobCounter, 40);
    
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
    EXPECT_NO_FATAL_FAILURE(WaitForCounter(&jobCounter));
    EXPECT_EQ(jobCounter, 0);

    DeinitializeJobSystem();
}

TEST(BorealisJobsTest, TestHierarchicalJobs)
{
    // Test jobs waiting on jobs waiting on jobs ...
}

#endif