#pragma once
#include "buddy.h"
#include "../common/shm_defines.h"

#define USE_SHARE_MEMORY

namespace smd {
class Alloc {
public:
	Alloc(void* ptr, size_t off_set, unsigned level)
		: m_basePtr(ptr)
		, m_offSet(off_set)
		, m_total(uint64_t(1) << level) {
#ifdef USE_SHARE_MEMORY
		m_buddy = SmdBuddyAlloc::buddy_new(level);
#endif
	}

	~Alloc() {
#ifdef USE_SHARE_MEMORY
		SmdBuddyAlloc::buddy_delete(m_buddy);
#endif
	}

	void* Malloc(size_t size) {
		m_used += size;
#ifdef USE_SHARE_MEMORY
		int64_t addr = SmdBuddyAlloc::buddy_alloc(m_buddy, size);
		assert(addr >= 0);
		return (void*)(addr + (int64_t)m_basePtr + m_offSet);
#else
		return malloc(size);
#endif
	}

	void Free(void* addr, size_t size) {
		m_used -= size;
#ifdef USE_SHARE_MEMORY
		SmdBuddyAlloc::buddy_free(m_buddy, (int)((size_t)addr - m_offSet - (int64_t)m_basePtr));
#else
		free(addr);
#endif
	}

	std::pair<size_t, size_t> GetUsage() const {
		return std::make_pair(m_used, m_total);
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
	const void* m_basePtr;
	const size_t m_offSet;
	const size_t m_total;
#ifdef USE_SHARE_MEMORY
	SmdBuddyAlloc::buddy* m_buddy;
#endif
	size_t m_used = 0;
};

} // namespace smd
