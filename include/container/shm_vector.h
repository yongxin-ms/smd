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
	ShmVector(Alloc& alloc, size_t capacity = 1)
		: ShmObj(alloc) {
		reserve(capacity);
	}

	ShmVector(const ShmVector& r)
		: ShmObj(r.m_alloc) {
		reserve(r.size());
		for (int i = 0; i < r.size(); i++) {
			auto& element = r[i];
			push_back(element);
		}
	}

	ShmVector& operator=(const ShmVector& r) {
		if (this != &r) {
			ShmVector(r).swap(*this);
		}
		return *this;
	}

	~ShmVector() {
		while (!empty()) {
			pop_back();
		}

		m_alloc.Free(m_start, capacity() + 1);
		m_finish	   = nullptr;
		m_endOfStorage = nullptr;
	}

	size_t size() const { return m_finish - m_start; }
	bool   empty() { return m_finish == m_start; }
	size_t capacity() const { return m_endOfStorage - m_start; }

	//访问元素相关
	reference operator[](size_t i) { return *m_start[i]; }
	const reference operator[](size_t i) const { return *m_start[i]; }
	reference front() { return *m_start[0]; }
	reference back() { return *m_start[size() - 1]; }
	pointer	  data() { return m_start; }

	void push_back(const value_type& value) {
		if (m_finish != m_endOfStorage) {
			*m_finish = m_alloc.New<value_type>(value);
			++m_finish;
		} else {
			reserve(capacity() + 2);
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

	// 设置容量
	void reserve(size_t new_capacity) {
		auto old_size = size();
		new_capacity  = GetSuitableCapacity(std::max(old_size, new_capacity));
		auto new_list	  = m_alloc.Malloc<value_type*>(new_capacity + 1);
		if (old_size > 0) {
			memcpy(new_list, m_start, sizeof(value_type*) * old_size);
			m_alloc.Free(m_start, capacity() + 1);
		}

		m_start		   = new_list;
		m_finish	   = &m_start[old_size];
		m_endOfStorage = &m_start[new_capacity];
	}

	// 改变vector中元素的数目
	void resize(size_t n, const value_type& val) {
		/*
		如果n比当前的vector元素数目要小，vector的容量要缩减到resize的第一个参数大小，并移除那些超出n的元素同时销毁他们。
		如果n比当前vector元素数目要大，在vector的末尾扩展需要的元素数目，如果第二个参数val指定了，扩展的新元素初始化为
			val的副本，否则按类型默认初始化。
		注意：如果n大于当前的vector的容量(是容量，并非vector的size)，将会引起自动内存分配。所以现有的pointer,references,iterators将会失效。
		*/
		const auto ori_size = size();
		if (n == ori_size)
			return;

		if (n > capacity()) {
			reserve(n);
		}

		if (n > ori_size) {
			// 在末尾添加元素
			for (auto i = ori_size; i < n; i++) {
				push_back(val);
			}
		} else {
			// 销毁部分元素
			for (auto i = n; i < ori_size; i++) {
				pop_back();
			}
		}
	}

private:
	void swap(ShmVector& x) {
		smd::swap(m_start, x.m_start);
		smd::swap(m_finish, x.m_finish);
		smd::swap(m_endOfStorage, x.m_endOfStorage);
	}

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
