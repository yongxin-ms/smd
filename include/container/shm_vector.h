﻿#pragma once
#include <string>
#include "../mem_alloc/alloc.h"

namespace smd {

template <class T>
class ShmVector {
	typedef T value_type;
	typedef T* iterator;
	typedef T& reference;
	typedef iterator pointer;

public:
	ShmVector(Alloc& alloc, size_t capacity = 0)
		: m_alloc(alloc) {
		m_capacity = m_alloc.GetExpectSize(capacity);
		m_start = m_alloc.New<T>(m_capacity);
		m_finish = m_start;
		m_endOfStorage = m_start + m_capacity;
	}

	//
	// 注意，析构函数里面不能调用clear()，需要使用者主动调用来回收共享内存
	//
	~ShmVector() {}

	size_t size() const { return m_finish - m_start; }
	bool empty() { return m_finish == m_start; }
	size_t capacity() const { return m_endOfStorage - m_start; }

	//访问元素相关
	reference operator[](size_t i) { return *(begin() + i); }
	reference front() { return *(begin()); }
	reference back() { return *(end() - 1); }
	pointer data() { return m_start; }

	void push_back(const value_type& value) {
		if (capacity() > size()) {
			*m_start[size()] = value;
			++m_finish;
		} else {
			auto new_capacity = m_alloc.GetExpectSize(capacity() * 2);
			auto new_start = m_alloc.Malloc<T>(new_capacity);
			auto size = size();
			memcpy(new_start, m_start, sizeof(T) * size);
			m_start = new_start;
			m_finish = m_start + size;
			m_endOfStorage = m_start + new_capacity;

			push_back(value);
		}
	}
	void pop_back() {
		--m_finish;
		auto d = m_finish;
		m_alloc.Delete(d);
	}

	void clear(bool deep = false) {
		while (!empty()) {
			pop_back();
		}

		if (deep) {
			m_alloc.Delete(m_start, capacity());
			m_finish = m_start;
			m_endOfStorage = m_start;
		}
	}

	iterator begin() { return m_start; }
	iterator end() { return m_finish; }

private:
	Alloc& m_alloc;
	T* m_start;
	T* m_finish;
	T* m_endOfStorage;
};

} // namespace smd