#pragma once
#include "shm_obj.h"

namespace smd {

template <class T>
class ShmVector : public ShmObj {
	typedef T		 value_type;
	typedef T*		 iterator;
	typedef T&		 reference;
	typedef iterator pointer;

public:
	ShmVector(Alloc& alloc, size_t capacity = 0)
		: ShmObj(alloc) {
		resize(GetSuitableCapacity(capacity));
	}

	~ShmVector() {
		while (!empty()) {
			pop_back();
		}

		m_alloc.Free(m_start, capacity());
		m_finish	   = nullptr;
		m_endOfStorage = nullptr;
	}

	size_t size() const { return m_finish - m_start; }
	bool   empty() { return m_finish == m_start; }
	size_t capacity() const { return m_endOfStorage - m_start + 1; }

	//访问元素相关
	reference operator[](size_t i) { return *m_start[i]; }
	reference front() { return *m_start[0]; }
	reference back() { return *m_start[size() - 1]; }
	pointer	  data() { return m_start; }

	void push_back(const value_type& value) {
		if (m_finish != m_endOfStorage) {
			*m_finish = m_alloc.New<value_type>(value);
			++m_finish;
		} else {
			resize(GetSuitableCapacity(capacity() * 2));
			push_back(value);
		}
	}

	void pop_back() {
		--m_finish;
		auto d = *m_finish;
		m_alloc.Delete(d);
	}

	void clear() {
		while (!empty()) {
			pop_back();
		}
	}

	iterator begin() { return m_start[0]; }
	iterator end() { return *m_finish; }

	void resize(size_t new_capacity) {
		auto old_size = size();
		new_capacity  = GetSuitableCapacity(std::max(old_size, new_capacity));
		auto new_list	  = m_alloc.Malloc<value_type*>(new_capacity);
		if (old_size > 0) {
			memcpy(new_list, m_start, sizeof(value_type*) * old_size);
			m_alloc.Free(m_start, capacity());
		}

		m_start		   = new_list;
		m_finish	   = &m_start[old_size];
		m_endOfStorage = &m_start[new_capacity - 1];
	}

private:
	size_t GetSuitableCapacity(size_t size) {
		if (size < 1)
			size = 1;
		return Utility::NextPowOf2(size);
	}

	void shrink_to_fit() {
		// 在此添加代码缩容
	}

private:
	value_type** m_start = nullptr;
	value_type** m_finish = nullptr;
	value_type** m_endOfStorage = nullptr;
};

} // namespace smd
