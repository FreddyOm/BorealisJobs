#include "scheduler.h"
#include "job-system.h"

namespace boost 
{
	namespace fibers 
	{
		namespace algo 
		{
			using namespace Borealis::Jobs;

			JobSystemScheduler::JobSystemScheduler(std::uint32_t thread_count)
			{

			}
			
			/// <summary>
			/// This fuction is called when a fiber is ready to be run. E.g., when this_fiber::yield() is called, 
			/// or when a fiber was blocked and is now ready to run again.
			/// </summary>
			/// <param name="ctx">Ptr to the fiber context.</param>
			/// <param name="props">Fiber properties, which contain the priority of the fiber.</param>
			void JobSystemScheduler::awakened(boostFiber* ctx, JobSchedulerWaitProperties& props) noexcept
			{
							
			}

			/// <summary>
			/// This function is called when a fiber is requested. 
			/// It picks the next fiber to be run by a given thread.
			/// </summary>
			/// <returns>A fiber to run or nullptr, if no fiber is available.</returns>
			JobSystemScheduler::boostFiber* JobSystemScheduler::pick_next() noexcept
			{
				// @TODO: Sync later

				// Check wait list				

				if (!g_wait_list.empty())
				{
					// Wait list spin lock
					ScopedSpinLock lock(g_wait_list_sl);

					bool isValidData = false;
					std::vector<WaitData>::iterator validWaitData = {};
					
					// Check wait list for ready jobs
					for (auto it = g_wait_list.begin(); it != g_wait_list.end(); ++it)
					{
						if (it->m_pCounter->load(std::memory_order_relaxed) <= it->m_desiredCount)
						{
							isValidData = true;
							validWaitData = it;
							break; // Break in order to make this work with multiple wait list elements!
						}
					}

					// If job is ready, remove wait data entry, return fiber and switch to waiting fiber!
					if (isValidData && validWaitData->m_isMainThreadJob == (std::this_thread::get_id() == g_mainThreadId))
					{
						context* fiber = validWaitData->m_pFiber;

						{
							g_wait_list.erase(validWaitData);								
						}
						return fiber;
					}
				}
				

				// If no wait list job is ready, pick the next free fiber from the global fiber pool!
				if (!g_fiber_pool.empty())
				{
					context* fiber = nullptr;

					{
						ScopedSpinLock lock(fiber_pool_sl);
						fiber = g_fiber_pool.front();
						g_fiber_pool.pop();
					}
					
					return fiber;
				}

				return nullptr;
			}


			/// <summary>
			/// Is called to check if fibers can be aquired by the scheduler.
			/// </summary>
			/// <returns>True if there is at least one ready fiber, false if there is no ready fiber.</returns>
			bool JobSystemScheduler::has_ready_fibers() const noexcept
			{
				return !g_fiber_pool.empty();
			}

			/// <summary>
			/// Is called when the fibers properties are changed.
			/// </summary>
			/// <param name="ctx"></param>
			/// <param name="props"></param>
			void JobSystemScheduler::property_change(boostFiber* ctx, JobSchedulerWaitProperties& props) noexcept
			{
				/*if (props.PutOnWaitList)
				{
					ScopedSpinLock lock(g_wait_list_sl);
					g_wait_list.push_back();
				}*/
			}

			void JobSystemScheduler::suspend_until(std::chrono::steady_clock::time_point const& time_point) noexcept
			{
				
			}

			void JobSystemScheduler::notify() noexcept
			{

			}


		}
	}
}