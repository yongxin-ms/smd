#pragma once
#include "../common/log.h"
#include "buddy.h"

namespace smd {
class Alloc;

template <typename T>
class Pointer {
public:
	Pointer(int64_t addr)
		: m_offSet(addr) {}

	T& operator*(const Alloc& alloc) const;
	T* operator()(const Alloc& alloc) const;
	int64_t operator()() const { return m_offSet; }

private:
	int64_t m_offSet;
};

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
	Pointer<T> Malloc(size_t n = 1) {
		auto size = sizeof(T) * n;
		auto addr = _Malloc(size);
		// m_log.DoLog(Log::LogLevel::kDebug, "malloc: 0x%p:(%d)", ptr, size);
		return Pointer<T>(addr);
	}

	template <class T>
	void Free(Pointer<T>& p, size_t n = 1) {
		auto size = sizeof(T) * n;
		// m_log.DoLog(Log::LogLevel::kDebug, "free: 0x%p:(%d)", p, size);
		_Free(p(*this), size);
		p = nullptr;
	}

	template <class T, typename... P>
	Pointer<T> New(P&&... params) {
		auto t = Malloc<T>();
		::new (t) T(std::forward<P>(params)...);
		return t;
	}

	template <class T>
	void Delete(Pointer<T>& p) {
		p(*this)->~T();
		Free(p);
	}

	template <class T>
	void Delete(T*& p) {
		p->~T();
		auto ptr = ToPointer<T>(p);
		Free(ptr);
	}

	size_t GetUsed() const { return m_used; }
	const char* StorageBasePtr() const { return m_storagePtr; }

	template <class T>
	Pointer<T> null_ptr() const {
		return Pointer<T>(m_storagePtr);
	}

	template <class T>
	Pointer<T> ToPointer(void* p) const {
		int64_t offset = (const char*)p - m_storagePtr;
		return Pointer<T>(offset);
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

template <typename T>
T& Pointer<T>::operator*(const Alloc& alloc) const {
	return *(T*)(alloc.StorageBasePtr() + m_offSet);
}

template <typename T>
T* Pointer<T>::operator()(const Alloc& alloc) const {
	return (T*)(alloc.StorageBasePtr() + m_offSet);
}

} // namespace smd
