﻿#pragma once
#include <container/shm_pointer.h>
#include <container/shm_vector.h>
#include <container/shm_list.h>

namespace smd {

template <class Key>
class shm_hash;

template <class Key, class ListIterator>
class HashIterator {
public:
	HashIterator(size_t index, ListIterator it, shm_pointer<shm_hash<Key>> ptr)
		: bucket_index_(index)
		, iterator_(it)
		, container_(ptr){};

	HashIterator& operator++() {
		++iterator_;
		//如果前进一位后到达了list的末尾，则需要跳转到下一个有item的bucket的list
		if (iterator_ == container_->m_buckets[bucket_index_].end()) {
			for (;;) {
				if (bucket_index_ == container_->m_buckets.size() - 1) {
					*this = container_->end();
					break;
				} else {
					++bucket_index_;
					if (!container_->m_buckets[bucket_index_].empty()) { //此list不为空
						iterator_ = container_->m_buckets[bucket_index_].begin();
						break;
					}
				}
			}
		}
		return *this;
	}

	HashIterator operator++(int) {
		auto res = *this;
		++*this;
		return res;
	}

	Key& operator*() {
		return *iterator_;
	}
	Key* operator->() {
		return &(operator*());
	}

	bool operator==(const HashIterator<Key, ListIterator>& rhs) const {
		return bucket_index_ == rhs.bucket_index_ && iterator_ == rhs.iterator_ && container_ == rhs.container_;
	}

	bool operator!=(const HashIterator<Key, ListIterator>& rhs) const {
		return !(*this == rhs);
	}

public:
	size_t bucket_index_;
	ListIterator iterator_;
	shm_pointer<shm_hash<Key>> container_;
};

template <class Key>
class shm_hash {
	friend class HashIterator<Key, typename shm_list<Key>::iterator>;

public:
	typedef size_t size_type;
	typedef Key key_type;
	typedef HashIterator<Key, typename shm_list<key_type>::iterator> iterator;
	typedef typename shm_list<key_type>::iterator local_iterator;

	shm_hash(size_t bucket_count = 1)
		: m_buckets(m_prime_util.NextPrime(bucket_count)) {
		m_buckets.resize(m_buckets.capacity(), shm_list<key_type>());
	}

	~shm_hash() {
		m_buckets.clear();
	}

	bool empty() const {
		return m_size == 0;
	}

	size_t size() const {
		return m_size;
	}

	size_type bucket_count() const {
		return m_buckets.size();
	}

	size_type bucket_size(size_type i) const {
		return m_buckets[i].size();
	}

	size_type bucket(const key_type& key) const {
		return bucket_index(key);
	}

	float load_factor() const {
		return (float)size() / (float)bucket_count();
	}

	float max_load_factor() const {
		return m_max_load_factor;
	}

	void max_load_factor(float z) {
		m_max_load_factor = z;
	}

	void rehash(size_type n) {
		if (n <= m_buckets.size())
			return;
		shm_hash<Key> temp(next_prime(n));
		for (auto& val : *this) {
			temp.insert(val);
		}
		std::swap(*this, temp);
	}

	iterator begin() {
		size_type index = 0;
		for (; index != m_buckets.size(); ++index) {
			if (!(m_buckets[index].empty()))
				break;
		}
		if (index == m_buckets.size())
			return end();
		return iterator(index, m_buckets[index].begin(), g_alloc->ToShmPointer<shm_hash<Key>>(this));
	}

	iterator end() {
		return iterator(m_buckets.size() - 1, m_buckets[m_buckets.size() - 1].end(),
						g_alloc->ToShmPointer<shm_hash<Key>>(this));
	}

	local_iterator begin(size_type i) {
		return m_buckets[i].begin();
	}

	local_iterator end(size_type i) {
		return m_buckets[i].end();
	}

	iterator find(const key_type& key) {
		auto index = bucket_index(key);
		for (auto it = begin(index); it != end(index); ++it) {
			if (key == *it)
				return iterator(index, it, g_alloc->ToShmPointer<shm_hash<Key>>(this));
		}
		return end();
	}

	size_type count(const key_type& key) {
		auto it = find(key);
		return it == end() ? 0 : 1;
	}

	std::pair<iterator, bool> insert(const key_type& val) {
		if (!has_key(val)) {
			if (load_factor() > max_load_factor())
				rehash(next_prime(size()));
			auto index = bucket_index(val);
			m_buckets[index].push_front(val);
			++m_size;
			return std::pair<iterator, bool>(
				iterator(index, m_buckets[index].begin(), g_alloc->ToShmPointer<shm_hash<Key>>(this)), true);
		}
		return std::pair<iterator, bool>(end(), false);
	}

	iterator erase(iterator position) {
		--m_size;
		auto t = position++;
		auto index = t.bucket_index_;
		m_buckets[index].erase(t.iterator_);
		return position;
	}

	bool erase(const key_type& key) {
		auto it = find(key);
		if (it == end()) {
			return false;
		} else {
			erase(it);
			return true;
		}
	}

	void clear() {
		auto it_end = end();
		for (auto it = begin(); it != it_end;) {
			it = erase(it);
		}
	}

	void swap(shm_hash<Key>& x) {
		std::swap(m_buckets, x.m_buckets);
		std::swap(m_size, x.m_size);
		std::swap(m_max_load_factor, x.m_max_load_factor);
	}

private:
	size_type next_prime(size_type n) const {
		return m_prime_util.NextPrime(n);
	}

	size_type bucket_index(const key_type& key) const {
		return std::hash<key_type>()(key) % m_buckets.size();
	}

	bool has_key(const key_type& key) {
		auto& list = m_buckets[bucket_index(key)];
		for (auto it = list.begin(); it != list.end(); ++it) {
			if (key == *it)
				return true;
		}
		return false;
	}

private:
	shm_vector<shm_list<Key>> m_buckets;
	size_t m_size = 0;
	float m_max_load_factor = 0.0f;

	static util::PrimeUtil m_prime_util;
};

template <class Key>
util::PrimeUtil shm_hash<Key>::m_prime_util;

} // namespace smd
