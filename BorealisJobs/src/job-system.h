#pragma once
#include "config.h"
#include <vector>
#include "job.h"


namespace Borealis::Jobs
{
	void CreateFiberPool();
	void CreateThreadPool(int numOfThreads, std::thread*& workerThread);
	BOREALIS_API void InitializeJobSystem(int numOfThreads = -1);
	BOREALIS_API void DeinitializeJobSystem();

	BOREALIS_API void WaitForCounter(Counter* const cnt, const int desiredCount = 0);
	BOREALIS_API void WaitForCounterAndFree(Counter* const cnt, const int desiredCount = 0);

	BOREALIS_API void KickJob(Job& job);
	BOREALIS_API void KickJobs(Job* jobs, int jobCount);

	BOREALIS_API void KickMainThreadJob(Job& job);
	BOREALIS_API void KickMainThreadJobs(Job* jobs, int jobCount);
	
	// @TODO: Implement specifically for main thread to preferably 
	// pick main thread jobs.
	
	// BOREALIS_API void KickMainThreadJob(Job job);
	// BOREALIS_API void KickMainThreadJobs(Job* job, int jobCount);

	void RunThread();
	void UpdateWaitData();
}
