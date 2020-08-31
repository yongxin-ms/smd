#pragma once
#include "../mem_alloc/alloc.h"

namespace smd {

class Alloc;
extern Alloc* g_alloc;

enum : int64_t {
	shm_nullptr = -1,
};

template <typename T>
class ShmPointer {
public:
	ShmPointer(int64_t addr = shm_nullptr)
		: m_offSet(addr) {}
	ShmPointer(const ShmPointer&) = default;
	ShmPointer& operator=(const ShmPointer&) = default;

	T& operator*() const;
	T* operator->() const;
	T* operator&() const { return (T*)(g_alloc->StorageBasePtr() + m_offSet); }

	int64_t operator()() const { return m_offSet; }
	bool operator==(const ShmPointer& r) const { return m_offSet == r.m_offSet; }
	bool operator!=(const ShmPointer& r) const { return m_offSet != r.m_offSet; }

	int64_t operator-(const ShmPointer& r) const { return (m_offSet - r.m_offSet) / sizeof(T); }
	ShmPointer operator+(int n) const {
		ShmPointer tmp(*this);
		tmp.m_offSet += n * sizeof(T);
		return tmp;
	}

	T& operator[](int n) {
		ShmPointer tmp(*this);
		tmp.m_offSet += n * sizeof(T);
		return *tmp;
	}

	const T& operator[](int n) const {
		ShmPointer tmp(*this);
		tmp.m_offSet += n * sizeof(T);
		return *tmp;
	}

	ShmPointer& operator++() {
		m_offSet += sizeof(T);
		return *this;
	}

	ShmPointer operator++(int) {
		ShmPointer tmp(*this);
		m_offSet += sizeof(T);
		return tmp;
	}

	ShmPointer& operator--() {
		m_offSet -= sizeof(T);
		return *this;
	}

	ShmPointer operator--(int) {
		ShmPointer tmp(*this);
		m_offSet -= sizeof(T);
		return tmp;
	}

private:
	int64_t m_offSet;
};

template <typename T>
T& ShmPointer<T>::operator*() const {
	return *((T*)(g_alloc->StorageBasePtr() + m_offSet));
}

template <typename T>
T* ShmPointer<T>::operator->() const {
	return (T*)(g_alloc->StorageBasePtr() + m_offSet);
}

} // namespace smd
