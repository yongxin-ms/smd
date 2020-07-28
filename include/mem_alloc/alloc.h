﻿#pragma once
#include "../common/log.h"
#include "buddy.h"

namespace smd {

class Alloc {
public:
	Alloc(Log& log, void* ptr, size_t off_set, unsigned level)
		: m_log(log)
		, m_basePtr((const char*)ptr + off_set) {
		m_buddy = (SmdBuddyAlloc::buddy*)m_basePtr;
		m_storagePtr = m_basePtr + SmdBuddyAlloc::get_index_size(level);
	}

	void CreateNew(unsigned level) { m_buddy = SmdBuddyAlloc::buddy_new(m_basePtr, level); }

	template <class T>
	T* Malloc(size_t n = 1) {
		auto size = sizeof(T) * n;
		auto ptr  = (T*)_Malloc(size);
		//m_log.DoLog(Log::LogLevel::kDebug, "malloc: 0x%p:(%d)", ptr, size);
		return ptr;
	}

	template <class T>
	void Free(T*& p, size_t n = 1) {
		auto size = sizeof(T) * n;
		//m_log.DoLog(Log::LogLevel::kDebug, "free: 0x%p:(%d)", p, size);
		_Free(p, size);
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

		m_log.DoLog(Log::LogLevel::kDebug, "malloc: 0x%08x:(%llu)", addr, size);

		m_used += size;
		return (void*)(addr + m_storagePtr);
	}

	void _Free(void* addr, size_t size) {
		auto vir_addr = (int)((const char*)addr - m_storagePtr);
		m_log.DoLog(Log::LogLevel::kDebug, "free: 0x%08x:(%llu)", vir_addr, size);

		m_used -= size;
		SmdBuddyAlloc::buddy_free(m_buddy, vir_addr);
	}

private:
	Log&				  m_log;
	const char*			  m_basePtr;
	SmdBuddyAlloc::buddy* m_buddy;
	size_t				  m_used = 0;
	const char*			  m_storagePtr;
};

} // namespace smd
