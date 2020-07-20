#pragma once
#include "buddy.h"

namespace smd {
class Alloc {
public:
	Alloc(void* ptr, size_t off_set, unsigned level)
		: m_basePtr(ptr)
		, m_offSet(off_set) {
		m_buddy = SmdBuddyAlloc::buddy_new((void*)((const char*)ptr + off_set), level);
	}

	void* Malloc(size_t size) {
		int64_t addr = SmdBuddyAlloc::buddy_alloc(m_buddy, size); 
		if (addr < 0) {
			assert(false);
			return nullptr;
		}

		m_used += size;
		return (void*)(addr + (int64_t)m_basePtr + m_offSet);
	}

	void Free(void* addr, size_t size) {
		m_used -= size;
		SmdBuddyAlloc::buddy_free(m_buddy, (int)((size_t)addr - m_offSet - (int64_t)m_basePtr));
	}

	size_t GetUsage() const {
		return m_used;
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
	SmdBuddyAlloc::buddy* m_buddy;
	size_t m_used = 0;
};

} // namespace smd
