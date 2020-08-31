#pragma once

namespace smd {

//*********** [shm_pair] ****************
template <class T1, class T2>
struct shm_pair {
public:
	typedef T1 first_type;
	typedef T2 second_type;

public:
	T1 first;
	T2 second;

public:
	shm_pair() {}
	template <class U, class V>
	shm_pair(const shm_pair<U, V>& pr);
	shm_pair(const first_type& a, const second_type& b);
	shm_pair& operator=(const shm_pair& pr);
	void swap(shm_pair& pr);

public:
	template <class T1, class T2>
	friend bool operator==(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs);
	template <class T1, class T2>
	friend bool operator!=(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs);
	template <class T1, class T2>
	friend bool operator<(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs);
	template <class T1, class T2>
	friend bool operator<=(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs);
	template <class T1, class T2>
	friend bool operator>(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs);
	template <class T1, class T2>
	friend bool operator>=(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs);
	template <class T1, class T2>
	friend void swap(shm_pair<T1, T2>& x, shm_pair<T1, T2>& y);
};

template <class T1, class T2>
template <class U, class V>
shm_pair<T1, T2>::shm_pair(const shm_pair<U, V>& pr)
	: first(pr.first)
	, second(pr.second) {}

template <class T1, class T2>
shm_pair<T1, T2>::shm_pair(const first_type& a, const second_type& b)
	: first(a)
	, second(b) {}

template <class T1, class T2>
shm_pair<T1, T2>& shm_pair<T1, T2>::operator=(const shm_pair<T1, T2>& pr) {
	if (this != &pr) {
		first = pr.first;
		second = pr.second;
	}
	return *this;
}

template <class T1, class T2>
void shm_pair<T1, T2>::swap(shm_pair<T1, T2>& pr) {
	swap(first, pr.first);
	swap(second, pr.second);
}

template <class T1, class T2>
bool operator==(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs) {
	return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <class T1, class T2>
bool operator!=(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs) {
	return !(lhs == rhs);
}

template <class T1, class T2>
bool operator<(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs) {
	return lhs.first < rhs.first || (!(rhs.first < lhs.first) && lhs.second < rhs.second);
}

template <class T1, class T2>
bool operator<=(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs) {
	return !(rhs < lhs);
}

template <class T1, class T2>
bool operator>(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs) {
	return rhs < lhs;
}

template <class T1, class T2>
bool operator>=(const shm_pair<T1, T2>& lhs, const shm_pair<T1, T2>& rhs) {
	return !(lhs < rhs);
}

template <class T1, class T2>
void swap(shm_pair<T1, T2>& x, shm_pair<T1, T2>& y) {
	x.swap(y);
}

// ******* [make_pair] ************
template <class U, class V>
shm_pair<U, V> make_pair(const U& u, const V& v) {
	return shm_pair<U, V>(u, v);
}

} // namespace smd
