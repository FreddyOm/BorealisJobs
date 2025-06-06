#include "job-system.h"
#include "scoped-spinlock.h"
#include "spinlock.h"
#include "scheduler.h"

#include <assert.h>
#include <thread>
#include <mutex>
#include <unordered_map>


using namespace boost::fibers;

namespace Borealis::Jobs
{
	// ------------------ General data ------------------
	std::thread::id g_mainThreadId{};
	boost::fibers::fiber g_mainFiber{};
	std::atomic<bool> g_runThreads(true);
	
	// ------------------ Thread and Fiber data ------------------

	std::vector<std::thread*> g_worker_threads = {};
	std::queue<boost::fibers::context*> g_fiber_pool{};

	// ------------------ Job queues ------------------

	std::queue<Job> g_job_queue_high;
	std::queue<Job> g_job_queue_normal;
	std::queue<Job> g_job_queue_low;

	// ------------------ Spinlocks ------------------

	SpinLock thread_fibers_sl{};
	SpinLock job_queue_low_sl{};
	SpinLock job_queue_normal_sl{};
	SpinLock job_queue_high_sl{};
	SpinLock main_thread_job_queue_sl{};
	SpinLock fiber_pool_sl{};
	SpinLock g_wait_list_sl{};
	SpinLock schedule_list_sl{};

	// ------------------ Wait data ------------------

	std::vector<WaitData> g_wait_list{};
	std::unordered_map<boost::fibers::context*, WaitData> schedule_list{};
	

#pragma region Interface functions

	/// <summary>
	/// Deinitializes the job system and clears all the resources.
	/// </summary>
	void DeinitializeJobSystem()
	{
		printf("Deinitializing job system from thread %d\n", GetCurrentThreadId());

		g_runThreads.store(false, std::memory_order_release);

		Sleep(1);

		if(IsThreadAFiber())
			ConvertFiberToThread(); // @TODO: Which thread will this be and will it be joined soon?

		// Join and clear the worker threads
		{
			for (auto* thread : g_worker_threads)
			{
				if (std::this_thread::get_id() != thread->get_id() && thread->joinable())
				{
					thread->join();
				}
			}

			g_worker_threads.clear();
		}
		
		// Clear the fiber pool and delete all fibers in the pool
		{
			ScopedSpinLock lock(fiber_pool_sl);
			while (g_fiber_pool.size() > 0)
			{
				LPVOID fiber = g_fiber_pool.front();
				g_fiber_pool.pop();
				DeleteFiber(fiber);
			}
		}

		// Clear the wait list and delete all fibers in the wait list
		{
			ScopedSpinLock lock(g_wait_list_sl);
			for(int i = 0; i < g_wait_list.size(); ++i)
			{
				DeleteFiber(g_wait_list[i].m_pFiber);
			}

			g_wait_list.clear();
		}

		// Delete all scheduled fibers and clear the list
		{
			ScopedSpinLock lock(schedule_list_sl);
			for (auto& kvp : schedule_list)
			{
				DeleteFiber(kvp.second.m_pFiber);
			}
			schedule_list.clear();
		}
		
		g_thread_fibers.clear();

		g_job_queue_high.clear();
		g_job_queue_normal.clear();
		g_job_queue_low.clear();
	}


	/// <summary>
	/// Initializes the job system 
	/// </summary>
	/// <param name="numOfThreads"></param>
	void InitializeJobSystem(size_t numThreads)
	{
		if (numThreads == 0) { return; }
		unsigned int hardwareThreads = std::thread::hardware_concurrency();

		// Define number of threads
		if (numThreads < 1 || numThreads > hardwareThreads)
			numThreads = hardwareThreads - 1;
		
		printf("Number of logical cpu cores: %i\n", std::thread::hardware_concurrency());
		printf("Number of worker threads: %i\n", numThreads);

		// Store true to enable the infinite working routine on each thread
		g_runThreads.store(true, std::memory_order_relaxed);
		
		boost::fibers::use_scheduling_algorithm<boost::fibers::algo::JobSystemScheduler>();

		CreateFiberPool(); // and reserve wait list to the maximum number of possible fibers
		CreateThreadPool(numThreads);

	}

	

	/// <summary>
	/// Schedules a job to be executed by the worker threads.
	/// </summary>
	/// <param name="job">The job to be executed.</param>
	void KickJob(Job& job)
	{
		switch (job.m_Priority)
		{
			case Priority::HIGH:
			{
				ScopedSpinLock lock(job_queue_high_sl);
				g_job_queue_high.emplace(std::move(job));
				break;
			}		
			case Priority::NORMAL:
			{
				ScopedSpinLock lock(job_queue_normal_sl);
				g_job_queue_normal.emplace(std::move(job));
				break;
			}
			case Priority::LOW:
			{
				ScopedSpinLock lock(job_queue_low_sl);
				g_job_queue_low.emplace(std::move(job));
				break;
			}
		}
	}

	/// <summary>
	/// Schedules a bunch of jobs to be executed by the worker threads.
	/// </summary>
	/// <param name="jobs">A pointer to the job array.</param>
	/// <param name="jobCount">The amount of jobs to be scheduled. Must be the size of the referenced job array.</param>
	void KickJobs(Job* jobs, int jobCount)
	{
		for (int i = 0; i < jobCount; ++i)
		{
			switch (jobs[i].m_Priority)
			{
				case Priority::HIGH:
				{
					ScopedSpinLock lock(job_queue_high_sl);
					g_job_queue_high.emplace(std::move(jobs[i]));
					break;
				}
				case Priority::NORMAL:
				{
					ScopedSpinLock lock(job_queue_normal_sl);
					g_job_queue_normal.emplace(std::move(jobs[i]));
					break;
				}
				case Priority::LOW:
				{
					ScopedSpinLock lock(job_queue_low_sl);
					g_job_queue_low.emplace(std::move(jobs[i]));
					break;
				}
			}
		}
	}

	/// <summary>
	/// Schedules a job to be executed by the main thread. 
	/// Do not use this extensively or the performance will be similar to single core performance plus overhead!!
	/// </summary>
	/// <param name="job">The job to be executed on the main thread.</param>
	void KickMainThreadJob(Job& job)
	{
		ScopedSpinLock lock(main_thread_job_queue_sl);
		g_main_thread_job_queue.push_back(std::move(job));
	}

	/// <summary>
	/// Schedules multiple jobs to be executed by the main thread. 
	/// Do not use this extensively or the performance will be similar to single core performance plus overhead!!
	/// </summary>
	/// <param name="jobs">The jobs to be executed on the main thread.</param>
	/// <param name="jobCount">The amount of jobs to be executed on the main thread.</param>
	void KickMainThreadJobs(Job* jobs, int jobCount)
	{
		ScopedSpinLock lock(main_thread_job_queue_sl);

		for (int i = 0; i < jobCount; ++i)
		{
			g_main_thread_job_queue.push_back(std::move(jobs[i]));
		}
	}

	

	/// <summary>
	/// Waits for the counter to become the desired count (or by default 0) and puts the job onto the wait list during meantime.
	/// This call is the synchronisation point in the execution flow.
	/// </summary>
	/// <param name="cnt">The counter to wait on.</param>
	/// <param name="desiredCount">The desired count the counter needs to reach in order to continue execution.</param>
	void WaitForCounter(Counter* const cnt, const int desiredCount)
	{
		if (cnt->load(std::memory_order_consume) > desiredCount)
		{
			// Fetch new fiber
			LPVOID fiber = GetFiber();	
			assert(fiber != nullptr);
			
			// Schedule for wait list!
			{
				ScopedSpinLock lock(schedule_list_sl);
				schedule_list.emplace(fiber, std::move(WaitData(GetCurrentFiber(), cnt, desiredCount)));
			}

			SwitchToFiber(fiber);
		}
	}

	/// <summary>
	/// Waits for the counter to become the desired count (or by default 0) and puts the job onto the wait list during meantime.
	/// When done, this function frees the heap allocated counter. This call is the synchronisation point in the execution flow.
	/// </summary>
	/// <param name="cnt">The counter to wait on. Expected to be heap allocated!</param>
	/// <param name="desiredCount">The desired count the counter needs to reach in order to continue execution.</param>
	void WaitForCounterAndFree(Counter* const cnt, const int desiredCount)
	{
		if (cnt->load(std::memory_order_consume) > desiredCount)
		{
			// Fetch new fiber
			LPVOID fiber = GetFiber();
			assert(fiber != nullptr);
		
			// Schedule for wait list!
			{
				ScopedSpinLock lock(schedule_list_sl);
				schedule_list.emplace(fiber, std::move(WaitData(this_fiber, cnt, desiredCount)));
			}

			// Switch to new fiber
			SwitchToFiber(GetFiber());
		}

		delete cnt;
	}

#pragma endregion Interface functions

#pragma region Internal functions

	void CreateThreadPool(size_t numThreads)
	{
		
	}

	void FiberRoutine()
	{
		Job job;

		// Fetch jobs and execute them
		while (g_runThreads.load())
		{

			// If wait list job is ready, execute it first!
			if (!g_wait_list.empty())
			{
				job.m_EntryPoint(job.m_Param);
				continue;
			}


			// If job is available, get access to the job and execute its function
			if (!g_job_queue_high.empty())
			{
				ScopedSpinLock lock(job_queue_high_sl);

				job = std::move(g_job_queue_high.front());
				g_job_queue_high.pop();

				job.m_EntryPoint(job.m_Param);
				continue;
			}

			if (!g_job_queue_normal.empty())
			{
				ScopedSpinLock lock(job_queue_normal_sl);

				job = std::move(g_job_queue_normal.front());
				g_job_queue_normal.pop();

				job.m_EntryPoint(job.m_Param);
				continue;
			}

			if (!g_job_queue_low.empty())
			{
				ScopedSpinLock lock(job_queue_low_sl);

				job = std::move(g_job_queue_low.front());
				g_job_queue_low.pop();

				job.m_EntryPoint(job.m_Param);
				continue;
			}
		}
	}

	void ThreadRoutine()
	{
		
	}

#pragma endregion Internal functions

}
