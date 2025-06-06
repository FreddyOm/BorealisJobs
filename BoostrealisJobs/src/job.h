#pragma once
#include "scoped-spinlock.h"
#include "spinlock.h"
#include <string>
#include <boost/fiber/all.hpp>

namespace Borealis::Jobs
{
	typedef void JobReturnType;
	typedef JobReturnType(*JobEntryPoint)(uintptr_t args);
	typedef std::atomic<int> Counter;

#define PARALLEL_JOB(entryPoint, priority, ...) Job(entryPoint, priority, #entryPoint, __VA_ARGS__)
#define JOB(entryPoint, counter, priority, ...) Job(entryPoint, counter, priority, #entryPoint, __VA_ARGS__)

	enum class alignas(4) Priority : short
	{
		LOW = 0,
		NORMAL = 1,
		HIGH = 2,
		CRITICAL = 3,
	};

	/// <summary>
	/// A structure used to describe a job which should be executed 
	/// at a given point in time.
	/// </summary>
	struct Job
	{
		JobEntryPoint m_EntryPoint = nullptr;		// 64 bytes
		boost::fibers::context* m_Fiber = nullptr;	// 8 bytes
		uintptr_t m_Param = NULL;					// 8 bytes
		Counter* m_pCounter = nullptr;				// 8 bytes

		unsigned int m_DesiredCount = 0;			// 4 bytes
		Priority m_Priority = (Priority)1;			// 4 bytes

		std::string m_FunctionName = std::string("");

		Job()
			: m_EntryPoint(nullptr)
		{ }

		Job(void* ep, Priority pr, std::string functionName, uintptr_t args = 0)
			: m_EntryPoint(static_cast<JobEntryPoint>(ep)), m_Param(args), m_Priority(pr), m_FunctionName(functionName)
		{ }

		Job(void* ep, Counter* pCnt, Priority pr, std::string functionName, uintptr_t args = 0)
			: m_EntryPoint(static_cast<JobEntryPoint>(ep)), m_Param(args), m_pCounter(pCnt), m_Priority(pr), m_FunctionName(functionName)
		{ }

		/// <summary>
		/// Explicitly copies the job - only used for debug purposes currently.
		/// </summary>
		/// <returns>A copy of the job description.</returns>
		Job Copy() const
		{
#ifndef _DEBUG
		   static_assert(false, "Copying fibers in debug mode is not allowed at the moment!")
#endif
			Job cpy(m_EntryPoint, m_Priority, m_FunctionName, m_Param);
			cpy.m_DesiredCount = m_DesiredCount;
			cpy.m_Fiber = m_Fiber;
			cpy.m_pCounter = m_pCounter;

			return cpy;
		}

		// Copy semantics
		Job(const Job& other) = delete;
		Job& operator=(const Job& other) = delete;

		// Move semantics
		Job(Job&& other) noexcept
			: m_EntryPoint(other.m_EntryPoint)
			, m_Fiber(other.m_Fiber)
			, m_Param(other.m_Param)
			, m_pCounter(other.m_pCounter)
			, m_DesiredCount(other.m_DesiredCount)
			, m_Priority(other.m_Priority)
			, m_FunctionName(other.m_FunctionName)
		{
			other.m_EntryPoint = nullptr;
			other.m_Fiber = nullptr;
			other.m_Param = NULL;
			other.m_pCounter = nullptr;
			other.m_DesiredCount = 0;
			other.m_Priority = Priority::NORMAL;
			other.m_FunctionName = "";
		}

		Job& operator=(Job&& other) noexcept
		{
			m_EntryPoint = other.m_EntryPoint;
			m_Fiber = other.m_Fiber;
			m_Param = other.m_Param;
			m_pCounter = other.m_pCounter;
			m_DesiredCount = other.m_DesiredCount;
			m_Priority = other.m_Priority;
			m_FunctionName = other.m_FunctionName;

			other.m_EntryPoint = nullptr;
			other.m_Fiber = nullptr;
			other.m_Param = NULL;
			other.m_pCounter = nullptr;
			other.m_DesiredCount = 0;
			other.m_Priority = Priority::NORMAL;
			other.m_FunctionName = "";

			return *this;
		}
	};

	struct WaitData
	{
		boost::fibers::context* m_pFiber = {};
		Counter* m_pCounter = nullptr;
		int m_desiredCount = 0;
		bool m_isMainThreadJob = false;

		WaitData()
			: m_pFiber({}), m_pCounter(nullptr), m_desiredCount(0), m_isMainThreadJob(false)
		{
		}

		WaitData(boost::fibers::context* pFiber, Counter* _counter, int desiredCount, const bool isMainThreadJob = false)
			: m_pFiber(pFiber), m_pCounter(_counter), m_desiredCount(desiredCount), m_isMainThreadJob(std::this_thread::get_id() == g_mainThreadId)
		{
		}

		~WaitData() = default;

		bool operator !=(const WaitData& other)
		{
			return m_pFiber != other.m_pFiber;
		}

		bool operator ==(const WaitData& other)
		{
			return m_pFiber == other.m_pFiber;
		}
	};
}
