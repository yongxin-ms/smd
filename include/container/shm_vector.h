﻿#pragma once
#include <container/shm_pointer.h>

namespace smd {

template <class T>
class shm_vector {
	typedef T value_type;
	typedef shm_pointer<T> iterator;
	typedef T& reference;
	typedef iterator pointer;

public:
	shm_vector(size_t capacity = 1) {
		reserve(capacity);
	}

	shm_vector(const shm_vector& r) {
		reserve(r.size());
		for (size_t i = 0; i < r.size(); i++) {
			const auto& element = r[i];
			push_back(element);
		}
	}

	shm_vector& operator=(const shm_vector& r) {
		if (this != &r) {
			shm_vector(r).swap(*this);
		}
		return *this;
	}

	~shm_vector() {
		while (!empty()) {
			pop_back();
		}

		//实际分配的空间比容量大1.
		g_alloc->Free(m_start, capacity() + 1);
		m_finish = shm_nullptr;
		m_end_of_storage = shm_nullptr;
	}

	size_t size() const {
		return m_finish - m_start;
	}

	bool empty() {
		return m_finish == m_start;
	}

	size_t capacity() const {
		return m_end_of_storage - m_start;
	}

	//访问元素相关
	reference operator[](size_t i) {
		return *m_start[i];
	}

	reference operator[](size_t i) const {
		return *m_start[i];
	}

	reference front() {
		return **m_start;
	}

	reference back() {
		return *(m_start[size() - 1]);
	}

	void push_back(const value_type& value) {
		if (m_finish != m_end_of_storage) {
			auto new_element = g_alloc->New<value_type>(value);
			*m_finish = new_element;
			++m_finish;
		} else {
			reserve(capacity() + 2);
			push_back(value);
		}
	}

	void pop_back() {
		--m_finish;
		auto d = *m_finish;
		g_alloc->Delete(d);
	}

	void clear() {
		while (!empty()) {
			pop_back();
		}
	}

	// 设置容量
	void reserve(size_t new_capacity) {
		auto old_size = size();
		new_capacity = GetSuitableCapacity(std::max(old_size, new_capacity));

		//多分配一个，用来存放尾结点
		auto new_list = g_alloc->Malloc<shm_pointer<value_type>>(new_capacity + 1);
		if (old_size > 0) {
			memcpy(new_list.Ptr(), m_start.Ptr(), sizeof(shm_pointer<value_type>) * old_size);
			g_alloc->Free(m_start, capacity() + 1);
		}

		m_start = new_list;
		m_finish = m_start + old_size;
		m_end_of_storage = m_start + new_capacity;
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
	void swap(shm_vector& x) {
		std::swap(m_start, x.m_start);
		std::swap(m_finish, x.m_finish);
		std::swap(m_end_of_storage, x.m_end_of_storage);
	}

	size_t GetSuitableCapacity(size_t size) {
		if (size < 1)
			size = 1;
		return util::Utility::NextPowOf2(uint32_t(size));
	}

	void shrink_to_fit() {
		// 在此添加代码缩容
	}

private:
	shm_pointer<shm_pointer<value_type>> m_start = shm_nullptr;
	shm_pointer<shm_pointer<value_type>> m_finish = shm_nullptr;
	shm_pointer<shm_pointer<value_type>> m_end_of_storage = shm_nullptr;
};

} // namespace smd
