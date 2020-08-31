#pragma once
#include "buddy.h"
#include "../container/shm_pointer.h"
#include "../common/log.h"

namespace smd {

class Alloc {
public:
	Alloc(Log& log, void* ptr, size_t off_set, unsigned level, bool create_new)
		: m_log(log)
		, m_basePtr((const char*)ptr + off_set) {
		m_buddy = (SmdBuddyAlloc::buddy*)m_basePtr;
		m_storagePtr = m_basePtr + SmdBuddyAlloc::get_index_size(level);

		if (create_new) {
			m_buddy = SmdBuddyAlloc::buddy_new(m_basePtr, level);
		}
	}

	template <class T>
	ShmPointer<T> Malloc(size_t n = 1) {
		auto size = sizeof(T) * n;
		auto addr = _Malloc(size);
		// m_log.DoLog(Log::LogLevel::kDebug, "malloc: 0x%p:(%d)", ptr, size);
		return ShmPointer<T>(addr);
	}

	template <class T>
	void Free(ShmPointer<T>& p, size_t n = 1) {
		auto size = sizeof(T) * n;
		// m_log.DoLog(Log::LogLevel::kDebug, "free: 0x%p:(%d)", p, size);
		_Free(p(), size);
		p = shm_nullptr;
	}

	template <class T, typename... P>
	ShmPointer<T> New(P&&... params) {
		auto t = Malloc<T>();
		::new (&t) T(std::forward<P>(params)...);
		return t;
	}

	template <class T>
	void Delete(ShmPointer<T>& p) {
		(&p)->~T();
		Free(p);
	}

	template <class T>
	void Delete(T*& p) {
		p->~T();
		auto ptr = ToShmPointer<T>(p);
		Free(ptr);
		p = nullptr;
	}

	size_t GetUsed() const { return m_used; }
	const char* StorageBasePtr() const { return m_storagePtr; }

	template <class T>
	ShmPointer<T> null_ptr() const {
		return ShmPointer<T>(m_storagePtr);
	}

	template <class T>
	ShmPointer<T> ToShmPointer(void* p) const {
		int64_t offset = (const char*)p - m_storagePtr;
		return ShmPointer<T>(offset);
	}

private:
	int64_t _Malloc(size_t size) {
		auto off_set = SmdBuddyAlloc::buddy_alloc(m_buddy, size);
		if (off_set < 0) {
			assert(false);
			return 0;
		}

		m_log.DoLog(Log::LogLevel::kDebug, "malloc: 0x%08x:(%llu)", off_set, size);
		m_used += size;
		return off_set;
	}

	void _Free(int64_t off_set, size_t size) {
		m_log.DoLog(Log::LogLevel::kDebug, "free: 0x%08x:(%llu)", off_set, size);
		m_used -= size;
		SmdBuddyAlloc::buddy_free(m_buddy, int(off_set));
	}

private:
	Log& m_log;
	const char* m_basePtr;
	SmdBuddyAlloc::buddy* m_buddy;
	size_t m_used = 0;
	const char* m_storagePtr;
};

} // namespace smd
