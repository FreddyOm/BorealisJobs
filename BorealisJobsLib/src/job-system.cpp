#include "job-system.h"
#include "scoped-spinlock.h"
#include "spinlock.h"

#include <assert.h>
#include <thread>
#include <queue>
#include <mutex>
#include <unordered_map>

#ifdef BOREALIS_WIN
#include <Windows.h>
#else
#error The Borealis job system is currently only available for Windows.
#endif

namespace Borealis::Jobs
{
	// ------------------ General data ------------------
	std::thread::id g_mainThreadId{};
	LPVOID g_mainFiber{};
	std::atomic<bool> g_runThreads(true);
	
	// ------------------ Thread and Fiber data ------------------

	std::vector<std::thread*> g_worker_threads = {};
	std::queue<LPVOID> fiber_pool{};

	std::unordered_map<std::thread::id, LPVOID> g_thread_fibers = {};

	// ------------------ Job queues ------------------

	std::deque<Job> g_job_queue_high{};
	std::deque<Job> g_job_queue_normal{};
	std::deque<Job> g_job_queue_low{};
	std::deque<Job> g_main_thread_job_queue{};

	// ------------------ Spinlocks ------------------

	SpinLock thread_fibers_sl{};
	SpinLock job_queue_low_sl{};
	SpinLock job_queue_normal_sl{};
	SpinLock job_queue_high_sl{};
	SpinLock main_thread_job_queue_sl{};
	SpinLock fiber_pool_sl{};
	SpinLock wait_list_sl{};
	SpinLock schedule_list_sl{};

	// ------------------ Wait data ------------------

	struct WaitData
	{
		LPVOID m_Fiber = nullptr;
		Counter* m_pCounter = nullptr;
		int m_desiredCount = 0;
		bool m_isMainThreadJob = false;

		WaitData()
			: m_Fiber(nullptr), m_pCounter(nullptr), m_desiredCount(0), m_isMainThreadJob(false)
		{ }

		WaitData(const LPVOID _fiber, Counter* _counter, int desiredCount, const bool isMainThreadJob = false)
			: m_Fiber(_fiber), m_pCounter(_counter), m_desiredCount(desiredCount), m_isMainThreadJob(std::this_thread::get_id() == g_mainThreadId)
		{ }

		~WaitData() = default;

		bool operator !=(const WaitData& other)
		{
			return m_Fiber != other.m_Fiber;
		}

		bool operator ==(const WaitData& other)
		{
			return m_Fiber == other.m_Fiber;
		}
	};

	std::vector<WaitData> g_wait_list{};
	std::unordered_map<LPVOID, WaitData> schedule_list{};
	

	/// <summary>
	/// Forces the execution flow to be paused here and continued at the same point but by the main thread!
	/// </summary>
	void ForceMainThreadExecution()
	{
		if (std::this_thread::get_id() == g_mainThreadId)
			return;	// We are already on the main thread -> Early exit!

		LPVOID fiber = GetFiber();
		assert(fiber != nullptr);

		// Schedule for wait list!
		{
			ScopedSpinLock lock(schedule_list_sl);
			Counter cnt = Counter(0);
			schedule_list.emplace(fiber, std::move(WaitData(GetCurrentFiber(), &cnt, 0, true)));
		}
		
		SwitchToFiber(fiber);

		printf("Continuing execution on thread %d\n", GetCurrentThreadId());
	}

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
			while (fiber_pool.size() > 0)
			{
				LPVOID fiber = fiber_pool.front();
				fiber_pool.pop();
				DeleteFiber(fiber);
			}
		}

		// Clear the wait list and delete all fibers in the wait list
		{
			ScopedSpinLock lock(wait_list_sl);
			for(int i = 0; i < g_wait_list.size(); ++i)
			{
				DeleteFiber(g_wait_list[i].m_Fiber);
			}

			g_wait_list.clear();
		}

		// Delete all scheduled fibers and clear the list
		{
			ScopedSpinLock lock(schedule_list_sl);
			for (auto& kvp : schedule_list)
			{
				DeleteFiber(kvp.second.m_Fiber);
			}
			schedule_list.clear();
		}
		
		g_thread_fibers.clear();

		g_job_queue_high.clear();
		g_job_queue_normal.clear();
		g_job_queue_low.clear();
	}

	/// <summary>
	/// Returns the next valid job that is available. Will prioritize as follows:
	/// 1. Main thread jobs
	/// 2. High priority jobs
	/// 3. Normal priority jobs
	/// 4. Low priority jobs
	/// </summary>
	/// <returns>TThe selected job from either of the four lists.</returns>
	Job GetNextJob()
	{
		Jobs::Job jobCpy;
		
		// MAIN THREAD queue
		// Handle main thread jobs seperately!
		if (std::this_thread::get_id() == g_mainThreadId)
		{
			ScopedSpinLock lock(main_thread_job_queue_sl);
			
			if (!g_main_thread_job_queue.empty()) 
			{
				jobCpy = std::move(g_main_thread_job_queue.front());
				g_main_thread_job_queue.pop_front();
			}

			return jobCpy;
		}
		
		// HIGH Priority queue
		{
			ScopedSpinLock lock(job_queue_high_sl);

			if (!g_job_queue_high.empty())
			{
				jobCpy = std::move(g_job_queue_high.front());
				g_job_queue_high.pop_front();
				return jobCpy;
			}
		}
		
		// NORMAL Priority queue
		{
			ScopedSpinLock lock(job_queue_normal_sl);

			if (!g_job_queue_normal.empty())
			{
				jobCpy = std::move(g_job_queue_normal.front());
				g_job_queue_normal.pop_front();
				return jobCpy;
			}
		}
		
		// LOW Priority queue
		{
			ScopedSpinLock lock(job_queue_low_sl);
			
			if (!g_job_queue_low.empty())
			{
				jobCpy = std::move(g_job_queue_low.front());
				g_job_queue_low.pop_front();
				return jobCpy;
			}
		}
		
		return jobCpy;
	}

	/// <summary>
	/// Checks the waitlist for waiting jobs and returns to those jobs that are done waiting.
	/// </summary>
	void CheckWaitList()
	{
		wait_list_sl.Acquire();

		bool waitListAlreadyReleased = false;

		if (!g_wait_list.empty())
		{
			bool isValidData = false;
			std::vector<WaitData>::iterator validWaitData = {};

			for (auto it = g_wait_list.begin(); it != g_wait_list.end(); ++it)
			{
				if (it->m_pCounter->load(std::memory_order_relaxed) <= it->m_desiredCount)
				{
					assert(it->m_pCounter->load(std::memory_order_relaxed) <= it->m_desiredCount);
					
					isValidData = true;
					validWaitData = it;
					break; // Break in order to make this work with multiple wait list elements!
				}
			}

			// If job is ready, remove wait data entry, return fiber and switch to waiting fiber!
			if (isValidData && validWaitData->m_isMainThreadJob == (std::this_thread::get_id() == g_mainThreadId))
			{
				LPVOID fiberToSwitchTo = validWaitData->m_Fiber;

				{
					ScopedSpinLock lock(fiber_pool_sl);
					
					g_wait_list.erase(validWaitData);
					wait_list_sl.Release();
					waitListAlreadyReleased = true;				// Store in FLS that we actually released the wait list spin lock!
					fiber_pool.push(GetCurrentFiber());
				}
				SwitchToFiber(fiberToSwitchTo);
			}
		}
		// If there was no job in the wait list we have to release the spin lock.
		// BUT: If we just came from a fiber we must not re-release the fiber! 
		// Otherwise there will be an invalid iterator somewhere else!
		if(!waitListAlreadyReleased)
			wait_list_sl.Release();
	}

	/// <summary>
	/// The infinite fiber routine that is being run on each fiber. The individual routine steps are the following:
	/// 1. Update the wait data and copy scheduled wait data to the wait list.
	/// 2. Check the wait list for entries and exectue them prioritized.
	/// 3. check if any job is available.
	/// 4. If at least one job is available, get the next job, validate it and execute it.
	/// </summary>
	VOID RunFiber()
	{
		while (g_runThreads)
		{
			{
				UpdateWaitData();	// @TODO: Try to somehow do this more elegantly!!

				CheckWaitList();

				if (g_job_queue_high.empty() && g_job_queue_normal.empty() && g_job_queue_low.empty() && g_main_thread_job_queue.empty())
					continue;

				Job jobCpy = GetNextJob();

				if (jobCpy.m_EntryPoint != nullptr)
				{
					// Valid job

					// New job -> Get new fiber!
					// Job will not have a fiber associated with it yet since this case is handled before!
					jobCpy.m_Fiber = GetCurrentFiber();
					jobCpy.m_EntryPoint(jobCpy.m_Param);

					// We might not want to associate a counter with a parallel job!
					if (jobCpy.m_pCounter != nullptr)
					{
						jobCpy.m_pCounter->fetch_sub(1);
					}
					
					jobCpy.m_Fiber = nullptr;
				}
			}
		}

		// Switch back to the initial RunThread Fiber
		if (std::this_thread::get_id() == g_mainThreadId)
			return;
		
		SwitchToFiber(g_thread_fibers.at(std::this_thread::get_id()));
	}

	/// <summary>
	/// Gets a fiber from the fiber pool.
	/// </summary>
	/// <returns>A new fiber to execute the nwext job on.</returns>
	LPVOID GetFiber()
	{
		LPVOID fiber;
		// Critical section!
		{
			ScopedSpinLock lock(fiber_pool_sl);
			assert(fiber_pool.size() != 0);

			fiber = fiber_pool.front();
			fiber_pool.pop();
		}
		return fiber;
	}

	/// <summary>
	/// The thread routine. Since it switches to the fiber routine mid-function, it doesn't have to be a loop.
	/// Also handles converting the fibers back to a thread.
	/// </summary>
	void RunThread()
	{
		LPVOID threadFiber = ConvertThreadToFiber(0);
		auto threadID = std::this_thread::get_id();

		// Store the fibers running in this thread inside this list.
		{
			ScopedSpinLock lock(thread_fibers_sl);
			g_thread_fibers.emplace(threadID, threadFiber);
		}

		SwitchToFiber(GetFiber());

		// Reconvert the fiber to a thread.
		ConvertFiberToThread();
		printf("Terminating Thread %d ...\n", GetCurrentThreadId());
	}

	/// <summary>
	/// Creates and initializes the global fiber pool.
	/// </summary>
	void CreateFiberPool()
	{
		for (int i = 0; i < NUM_FIBERS(); ++i)
		{
			LPVOID fiber = CreateFiber(1024, (LPFIBER_START_ROUTINE)&RunFiber, NULL);
			fiber_pool.push(fiber);
		}

		g_wait_list.reserve(fiber_pool.size());
	}

	/// <summary>
	/// Creates and initializes the thread pool and some dependant resources.
	/// </summary>
	/// <param name="numOfThreads">The amount of threads to spawn in the thread pool.</param>
	void CreateThreadPool(const int numOfThreads)
	{
		// Init map and array with the amount of worker threads to be spawned
		g_thread_fibers.reserve(numOfThreads);
		g_worker_threads.reserve(numOfThreads);

		// Set some global thread and fiber information
		g_mainThreadId = std::this_thread::get_id();

		std::thread* workerThread = nullptr;

		for (unsigned short t_index = 0; t_index < numOfThreads; ++t_index)
		{
			// Spawn threads
			workerThread = new std::thread(RunThread);
			auto hndl = workerThread->native_handle();

			// Add to list
			g_worker_threads.push_back(workerThread);
		}
	}

	/// <summary>
	/// Initializes the job system 
	/// </summary>
	/// <param name="numOfThreads"></param>
	void InitializeJobSystem(int numOfThreads)
	{
		if (numOfThreads == 0) { return; }
		unsigned int hardwareThreads = std::thread::hardware_concurrency();

		// Define number of threads
		if (numOfThreads < 1 || numOfThreads > hardwareThreads)
			numOfThreads = hardwareThreads - 1;
		
		printf("Number of logical cpu cores: %i\n", std::thread::hardware_concurrency());
		printf("Number of worker threads: %i\n", numOfThreads);

		// Store true to enable the infinite working routine on each thread
		g_runThreads.store(true, std::memory_order_relaxed);
		
		CreateFiberPool(); // and reserve wait list to the maximum number of possible fibers
		CreateThreadPool(numOfThreads);

		g_mainFiber = ConvertThreadToFiber(0);
	}

	/// <summary>
	/// Returns a fiber to the fiber pool.
	/// </summary>
	/// <param name="fiber"></param>
	void ReturnFiber(const LPVOID fiber)
	{
		// Critical section!
		{
			ScopedSpinLock lock(fiber_pool_sl);
			fiber_pool.push(fiber);
		}
	}

	/// <summary>
	/// Schedules a job to be executed by the worker threads.
	/// </summary>
	/// <param name="job">The job to be executed.</param>
	void KickJob(const Job& job)
	{
		switch (job.m_Priority)
		{
			case Priority::HIGH:
			{
				ScopedSpinLock lock(job_queue_high_sl);
				g_job_queue_high.push_back(job);
				break;
			}		
			case Priority::NORMAL:
			{
				ScopedSpinLock lock(job_queue_normal_sl);
				g_job_queue_normal.push_back(job);
				break;
			}
			case Priority::LOW:
			{
				ScopedSpinLock lock(job_queue_low_sl);
				g_job_queue_low.push_back(job);
				break;
			}
		}
	}

	/// <summary>
	/// Schedules a bunch of jobs to be executed by the worker threads.
	/// </summary>
	/// <param name="jobs">A pointer to the job array.</param>
	/// <param name="jobCount">The amount of jobs to be scheduled. Must be the size of the referenced job array.</param>
	void KickJobs(Job* const jobs, const int jobCount)
	{
		for (int i = 0; i < jobCount; ++i)
		{
			switch (jobs[i].m_Priority)
			{
				case Priority::HIGH:
				{
					ScopedSpinLock lock(job_queue_high_sl);
					g_job_queue_high.push_back(jobs[i]);
					break;
				}
				case Priority::NORMAL:
				{
					ScopedSpinLock lock(job_queue_normal_sl);
					g_job_queue_normal.push_back(jobs[i]);
					break;
				}
				case Priority::LOW:
				{
					ScopedSpinLock lock(job_queue_low_sl);
					g_job_queue_low.push_back(jobs[i]);
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
	void KickMainThreadJob(const Job& job)
	{
		ScopedSpinLock lock(main_thread_job_queue_sl);
		g_main_thread_job_queue.push_back(job);
	}

	/// <summary>
	/// Schedules multiple jobs to be executed by the main thread. 
	/// Do not use this extensively or the performance will be similar to single core performance plus overhead!!
	/// </summary>
	/// <param name="jobs">The jobs to be executed on the main thread.</param>
	/// <param name="jobCount">The amount of jobs to be executed on the main thread.</param>
	void KickMainThreadJobs(Job* const jobs, const int jobCount)
	{
		ScopedSpinLock lock(main_thread_job_queue_sl);

		for (int i = 0; i < jobCount; ++i)
		{
			g_main_thread_job_queue.push_back(std::move(jobs[i]));
		}
	}

	/// <summary>
	/// Checks if a job + fiber was recently pushed to be scheduled in the wait list. 
	/// If the fiber that scheduled the waitdata was already switched from, the wait data can be safely pushed to the wait list.
	/// !! ***Important: This function seems to be unnecessary overhead but is essential for avoiding race conditions of the fiber
	/// being not switched from yet but already finished in the wait list.*** !!
	/// </summary>
	void UpdateWaitData()
	{
		ScopedSpinLock lock(schedule_list_sl);
		if (schedule_list.empty()) 
		{
			return; 
		}

		LPVOID currentFiber = GetCurrentFiber();

		if (schedule_list.contains(currentFiber))
		{
			WaitData waitDataCpy = std::move(schedule_list[currentFiber]);
			schedule_list.erase(currentFiber);

			{
				ScopedSpinLock wl_lock(wait_list_sl);
				g_wait_list.push_back(std::move(waitDataCpy));
			}
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
				schedule_list.emplace(fiber, std::move(WaitData(GetCurrentFiber(), cnt, desiredCount)));
			}

			// Switch to new fiber
			SwitchToFiber(GetFiber());
		}

		delete cnt;
	}
}
