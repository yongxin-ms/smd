#pragma once
#include "../mem_alloc/alloc.h"

namespace smd {

class Alloc;
static Alloc* g_alloc = nullptr;

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

	T* Ptr() const {
		assert(m_offSet != shm_nullptr && m_offSet != 0);
		return (T*)(g_alloc->StorageBasePtr() + m_offSet);
	}

	T* operator->() const { return Ptr(); }
	T& operator*() const { return *Ptr(); }

	int64_t Raw() const { return m_offSet; }
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

} // namespace smd
