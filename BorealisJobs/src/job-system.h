#pragma once
#include "config.h"
#include <vector>
#include "job.h"


namespace Borealis::Jobs
{
	BOREALIS_API void InitializeJobSystem(int numOfThreads = -1);
	BOREALIS_API void DeinitializeJobSystem();

	BOREALIS_API void BusyWaitForCounter(Counter* const cnt, const int desiredCount = 0);
	BOREALIS_API void BusyWaitForCounterAndFree(Counter* const cnt, const int desiredCount = 0);

	BOREALIS_API void KickJob(Job job);

	/// <summary>
	/// Kick multiple jobs at once. Use this preferrably when possible since it 
	/// will lock the job queue only once to push all the jobs!
	/// </summary>
	/// <param name="jobs">A pointer to a job description array.</param>
	/// <param name="jobCount">The amount of jobs to push.</param>
	/// <returns></returns>
	BOREALIS_API void KickJobs(Job* jobs, int jobCount);
	
	// @TODO: Implement specifically for main thread to preferably 
	// pick main thread jobs.
	
	// BOREALIS_API void KickMainThreadJob(Job job);
	// BOREALIS_API void KickMainThreadJobs(Job* job, int jobCount);

	void RunThread();
	void UpdateWaitData();
}
