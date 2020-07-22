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

	template <class T>
	T* Malloc(size_t n = 1) {
		return (T*)_Malloc(sizeof(T) * n);
	}

	template <class T>
	void Free(T*& p, size_t n = 1) {
		_Free(p, sizeof(T) * n);
		p = nullptr;
	}

	template <class T>
	T* New(size_t n = 1) {
		auto t = Malloc<T>(n);
// 		for (int i = 0; i < n; i++) {
// 			(t+i)->T();
// 		}
		return t;
	}

	template <class T>
	void Delete(T*& p, size_t n = 1) {
		for (int i = 0; i < n; i++) {
			(p+i)->~T();
		}
		Free(p, n);
	}

	size_t GetUsed() const { return m_used; }

private:
	void* _Malloc(size_t size) {
		int64_t addr = SmdBuddyAlloc::buddy_alloc(m_buddy, size);
		if (addr < 0) {
			assert(false);
			return nullptr;
		}

		m_used += size;
		return (void*)(addr + (int64_t)m_basePtr + m_offSet);
	}

	void _Free(void* addr, size_t size) {
		m_used -= size;
		SmdBuddyAlloc::buddy_free(m_buddy, (int)((size_t)addr - m_offSet - (int64_t)m_basePtr));
	}

private:
	const void* m_basePtr;
	const size_t m_offSet;
	SmdBuddyAlloc::buddy* m_buddy;
	size_t m_used = 0;
};

} // namespace smd
