#include "job.h"

#include <boost/fiber/all.hpp>
#include <boost/lockfree/queue.hpp>

using Borealis::Jobs::Job;

namespace boost 
{
	namespace fibers 
	{
		namespace algo 
		{
			class JobSchedulerWaitProperties : public boost::fibers::fiber_properties {
			public:
				JobSchedulerWaitProperties(boost::fibers::context* ctx) :
					fiber_properties(ctx), 
					PutOnWaitList(false)
				{ }

				bool PutOnWaitList = false;
				
			};


			class JobSystemScheduler : public boost::fibers::algo::algorithm_with_properties< JobSchedulerWaitProperties >
			{
			private:

				typedef boost::fibers::scheduler::ready_queue_type readyQueue;
				typedef boost::fibers::context boostFiber;

			public:

				JobSystemScheduler(std::uint32_t thread_count);

				JobSystemScheduler(JobSystemScheduler const&) = delete;
				JobSystemScheduler(JobSystemScheduler&&) = delete;

				JobSystemScheduler& operator=(JobSystemScheduler const&) = delete;
				JobSystemScheduler& operator=(JobSystemScheduler&&) = delete;



				virtual void awakened(boostFiber* ctx, JobSchedulerWaitProperties& props) noexcept;
				virtual boostFiber* pick_next() noexcept;
				virtual bool has_ready_fibers() const noexcept;
				virtual void property_change(boostFiber* ctx, JobSchedulerWaitProperties& props) noexcept;
				void suspend_until(std::chrono::steady_clock::time_point const& time_point) noexcept;
				void notify() noexcept;
			};
		}
	}
}