#pragma once
#include <string>
#include "../mem_alloc/alloc.h"

namespace smd {

template <class T>
class ShmVector {
public:
	ShmVector(Alloc& alloc, size_t capacity = 0)
		: m_alloc(alloc) {
		m_capacity = m_alloc.GetExpectSize(capacity);
		m_element = m_alloc.New<T>(m_capacity);
		m_size = 0;
	}

	//
	// 注意，析构函数里面不能调用clear()，需要使用者主动调用来回收共享内存
	//
	~ShmVector() {}

	size_t size() const { return m_size; }
	bool empty() { return m_size > 0; }
	size_t capacity() const { return m_capacity; }

	void clear(bool deep = false) {}

private:
	Alloc& m_alloc;
	T* m_element;
	size_t m_capacity;
	size_t m_size;
};

} // namespace smd
