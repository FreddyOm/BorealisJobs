#pragma once
#include "config.h"
#include <vector>
#include "job.h"


namespace Borealis::Jobs
{
	typedef void* LPVOID;

	BOREALIS_API void InitializeJobSystem(int numOfThreads = -1);
	BOREALIS_API void DeinitializeJobSystem();

	BOREALIS_API void KickJob(Job& job);
	BOREALIS_API void KickJobs(Job* jobs, int jobCount);

	BOREALIS_API void KickMainThreadJob(Job& job);
	BOREALIS_API void KickMainThreadJobs(Job* jobs, int jobCount);

	BOREALIS_API void WaitForCounter(Counter* const cnt, const int desiredCount = 0);
	BOREALIS_API void WaitForCounterAndFree(Counter* const cnt, const int desiredCount = 0);

	// --------------------------------------------------------

	void ForceMainThreadExecution();
	Job	 GetNextJob();
	void CheckWaitList();
	VOID RunFiber();
	LPVOID GetFiber();
	void RunThread();
	void CreateFiberPool();
	void CreateThreadPool(int numOfThreads);
	void ReturnFiber(LPVOID fiber);
	void UpdateWaitData();
}
