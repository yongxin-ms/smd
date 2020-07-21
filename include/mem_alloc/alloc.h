#pragma once
#include "buddy.h"

namespace smd {
/*
**空间配置器，以变量数目为单位分配
*/
template <class T>
class allocator {
public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

public:
	T* allocate();
	T* allocate(size_t n);
	void deallocate(T* ptr);
	void deallocate(T* ptr, size_t n);

	void construct(T* ptr);
	void construct(T* ptr, const T& value);
	void destroy(T* ptr);
	void destroy(T* first, T* last);

private:
	const void* m_basePtr;
	const size_t m_offSet;
	SmdBuddyAlloc::buddy* m_buddy;
	size_t m_used = 0;
};

template <class T>
T* allocator<T>::allocate() {
	return static_cast<T*>(alloc::allocate(sizeof(T)));
}
template <class T>
T* allocator<T>::allocate(size_t n) {
	if (n == 0)
		return 0;
	return static_cast<T*>(alloc::allocate(sizeof(T) * n));
}
template <class T>
void allocator<T>::deallocate(T* ptr) {
	alloc::deallocate(static_cast<void*>(ptr), sizeof(T));
}
template <class T>
void allocator<T>::deallocate(T* ptr, size_t n) {
	if (n == 0)
		return;
	alloc::deallocate(static_cast<void*>(ptr), sizeof(T) * n);
}

template <class T>
void allocator<T>::construct(T* ptr) {
	new (ptr) T();
}
template <class T>
void allocator<T>::construct(T* ptr, const T& value) {
	new (ptr) T(value);
}
template <class T>
void allocator<T>::destroy(T* ptr) {
	ptr->~T();
}
template <class T>
void allocator<T>::destroy(T* first, T* last) {
	for (; first != last; ++first) {
		first->~T();
	}
}

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

	template<class T>
	T* New() {
		auto t = (T*)Malloc(sizeof(T));
		t->T();
	}

	template <class T>
	void Delete(T* p) {
		p->~T();
		Free(p, sizeof(T));
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
