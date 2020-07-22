#pragma once
#include <string>
#include "shm_string.h"
#include "shm_list.h"
#include "shm_vector.h"
#include "../mem_alloc/alloc.h"

namespace smd {

template <class T>

class HashIterator {
private:
	template <class Key>
	friend class ShmHash;

private:
	typedef Unordered_set<Key, Hash, KeyEqual, Allocator>* cntrPtr;
	size_t bucket_index_;
	ListIterator iterator_;
	cntrPtr container_;

public:
	ust_iterator(size_t index, ListIterator it, cntrPtr ptr);
	ust_iterator& operator++();
	ust_iterator operator++(int);
	Key& operator*() { return *iterator_; }
	Key* operator->() { return &(operator*()); }

private:
	template <class Key, class ListIterator, class Hash, class KeyEqual, class Allocator>
	friend bool operator==(const ust_iterator<Key, ListIterator, Hash, KeyEqual, Allocator>& lhs,
		const ust_iterator<Key, ListIterator, Hash, KeyEqual, Allocator>& rhs);
	template <class Key, class ListIterator, class Hash, class KeyEqual, class Allocator>
	friend bool operator!=(const ust_iterator<Key, ListIterator, Hash, KeyEqual, Allocator>& lhs,
		const ust_iterator<Key, ListIterator, Hash, KeyEqual, Allocator>& rhs);
};

template <class Key>
class ShmHash {
	typedef size_t size_type;
	typedef Key key_type;

public:
	enum {
		PRIME_LIST_SIZE = 28,
	};

	ShmHash(Alloc& alloc, const std::string& name, size_t bucket_count)
		: m_alloc(alloc)
		, m_name(alloc, name)
		, m_buckets(alloc) {}

	//
	// 注意，析构函数里面不能调用clear()，需要使用者主动调用来回收共享内存
	//
	~ShmHash() {}

	bool empty() const { return m_size == 0; }
	size_t size() const { return m_size; }
	size_type bucket_count() const { return m_buckets.size(); }
	size_type bucket_size(size_type i) const { return m_buckets[i].size(); }
	size_type bucket(const key_type& key) const { return bucket_index(key); }
	float load_factor() const { return (float)size() / (float)bucket_count(); }
	float max_load_factor() const { return m_max_load_factor; }
	void max_load_factor(float z) { m_max_load_factor = z; }
	void rehash(size_type n) {
		if (n <= m_buckets.size())
			return;
		ShmHash temp(m_alloc, m_name, next_prime(n));
		for (auto& val : *this) {
			temp.insert(val);
		}
		TinySTL::swap(*this, temp);
	}

private:
	size_type next_prime(size_type n) const {
		auto i = 0;
		for (; i != PRIME_LIST_SIZE; ++i) {
			if (n > m_prime_list[i])
				continue;
			else
				break;
		}
		i = (i == PRIME_LIST_SIZE ? PRIME_LIST_SIZE - 1 : i);
		return m_prime_list[i];
	}
	size_type bucket_index(const key_type& key) const { return haser()(key) % buckets_.size(); }
	bool has_key(const key_type& key) {
		auto& list = buckets_[bucket_index(key)];
		auto pred = std::bind(KeyEqual(), key, std::placeholders::_1);
		return TinySTL::find_if(list.begin(), list.end(), pred) != list.end();
	}

private:
	Alloc& m_alloc;
	ShmString m_name;

	ShmVector<ShmList<Key>> m_buckets;
	size_t m_size;
	float m_max_load_factor;

	size_t m_prime_list[PRIME_LIST_SIZE];
};

} // namespace smd
