#pragma once
#include "config.h"
#include <vector>
#include <queue>
#include "job.h"
#include <boost/fiber/all.hpp>


namespace Borealis::Jobs
{
	struct WaitData;

	BOREALIS_API void InitializeJobSystem(size_t numOfThreads = -1);
	BOREALIS_API void DeinitializeJobSystem();

	BOREALIS_API void KickJob(Job& job);
	BOREALIS_API void KickJobs(Job* jobs, int jobCount);

	BOREALIS_API void KickMainThreadJob(Job& job);
	BOREALIS_API void KickMainThreadJobs(Job* jobs, int jobCount);

	BOREALIS_API void WaitForCounter(Counter* const cnt, const int desiredCount = 0);
	BOREALIS_API void WaitForCounterAndFree(Counter* const cnt, const int desiredCount = 0);

	// --------------------------------------------------------

	void CreateFiberPool();
	void CreateThreadPool(size_t numThreads);
	void FiberRoutine();
	void ThreadRoutine();

	extern std::queue<boost::fibers::context*> g_fiber_pool;
	extern std::vector<WaitData> g_wait_list;
	extern std::thread::id g_mainThreadId;
	
	extern SpinLock fiber_pool_sl;
	extern SpinLock g_wait_list_sl;
	
}
