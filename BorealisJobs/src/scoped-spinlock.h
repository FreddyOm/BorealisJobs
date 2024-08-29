#pragma once
#include "spinlock.h"

namespace Borealis::Jobs 
{
	struct ScopedSpinLock
	{
		ScopedSpinLock(const SpinLock& _spinlock) noexcept
			: spinlock(&_spinlock)
		{
			spinlock->Acquire();
		}

		~ScopedSpinLock() noexcept
		{
			spinlock->Release();
		}

	private:
		SpinLock const* spinlock = nullptr;
	};
}
