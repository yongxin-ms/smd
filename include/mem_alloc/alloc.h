#pragma once
#include "../common/log.h"
#include "buddy.h"

namespace smd {

class Alloc {
public:
	Alloc(Log& log, void* ptr, size_t off_set, unsigned level)
		: m_log(log)
		, m_basePtr((const char*)ptr)
		, m_offSet(off_set) {
		m_buddy = SmdBuddyAlloc::buddy_new((m_basePtr + m_offSet), level);
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

	template <class T, typename... P>
	T* New(P&&... params) {
		auto t = Malloc<T>();
		::new (t) T(std::forward<P>(params)...);
		return t;
	}

	template <class T>
	void Delete(T*& p) {
		p->~T();
		Free(p);
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
		return (void*)(addr + m_basePtr + m_offSet);
	}

	void _Free(void* addr, size_t size) {
		m_used -= size;
		SmdBuddyAlloc::buddy_free(m_buddy, (int)((const char*)addr - m_offSet - m_basePtr));
	}

private:
	Log&				  m_log;
	const char*			  m_basePtr;
	const size_t		  m_offSet;
	SmdBuddyAlloc::buddy* m_buddy;
	size_t				  m_used = 0;
};

} // namespace smd
