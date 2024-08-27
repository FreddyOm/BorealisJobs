#pragma once
#include "spinlock.h"

namespace Borealis::Jobs 
{
	struct ScopedSpinLock
	{
		ScopedSpinLock(SpinLock& const _spinlock) noexcept
			: spinlock(&_spinlock)
		{
			spinlock->Acquire();
		}

		~ScopedSpinLock() noexcept
		{
			spinlock->Release();
			spinlock = nullptr;
		}

	private:
		SpinLock* spinlock = nullptr;
	};
}
