#pragma once
#include "buddy.hpp"
#include "../common/smd_defines.h"

#define USE_SHARE_MEMORY

namespace smd {
class Alloc {
public:
	Alloc() {
#ifdef USE_SHARE_MEMORY
		m_buddy = SmdBuddyAlloc::buddy_new(5);
#endif
	}

	~Alloc() {
#ifdef USE_SHARE_MEMORY
		SmdBuddyAlloc::buddy_delete(m_buddy);
#endif
	}

	SMD_POINTER Malloc(size_t size) {
#ifdef USE_SHARE_MEMORY
		SmdBuddyAlloc::buddy_alloc(m_buddy, size);
#else
		return (SMD_POINTER)malloc(size);
#endif
	}

	void Free(SMD_POINTER addr) {
#ifdef USE_SHARE_MEMORY
		SmdBuddyAlloc::buddy_free(m_buddy, (int)addr);
#else
		return free((void*)addr);
#endif
	}

	static size_t GetExpectSize(size_t size) {
		if (size <= 16)
			return 16;
		else if (size <= 64)
			return size_t(size * 1.5);
		else if (size <= 256)
			return size_t(size * 1.4);
		else if (size <= 1024)
			return size_t(size * 1.3);
		else
			return size_t(size * 1.2);
	}

private:
#ifdef USE_SHARE_MEMORY
	SmdBuddyAlloc::buddy* m_buddy;
#endif
};

} // namespace smd
