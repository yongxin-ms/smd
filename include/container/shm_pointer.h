#pragma once

namespace smd {

enum : int64_t {
	shm_nullptr = -1,
};

template <typename T>
class shm_pointer {
public:
	shm_pointer(int64_t addr = shm_nullptr)
		: m_offSet(addr) {}
	shm_pointer(const shm_pointer&) = default;
	shm_pointer& operator=(const shm_pointer&) = default;

	T* Ptr() const;
	T* operator->() const { return Ptr(); }
	T& operator*() const { return *Ptr(); }

	int64_t Raw() const { return m_offSet; }
	bool operator==(const shm_pointer& r) const { return m_offSet == r.m_offSet; }
	bool operator!=(const shm_pointer& r) const { return m_offSet != r.m_offSet; }

	int64_t operator-(const shm_pointer& r) const { return (m_offSet - r.m_offSet) / sizeof(T); }
	shm_pointer operator+(int n) const {
		shm_pointer tmp(*this);
		tmp.m_offSet += n * sizeof(T);
		return tmp;
	}

	T& operator[](int n) {
		shm_pointer tmp(*this);
		tmp.m_offSet += n * sizeof(T);
		return *tmp;
	}

	const T& operator[](int n) const {
		shm_pointer tmp(*this);
		tmp.m_offSet += n * sizeof(T);
		return *tmp;
	}

	shm_pointer& operator++() {
		m_offSet += sizeof(T);
		return *this;
	}

	shm_pointer operator++(int) {
		shm_pointer tmp(*this);
		m_offSet += sizeof(T);
		return tmp;
	}

	shm_pointer& operator--() {
		m_offSet -= sizeof(T);
		return *this;
	}

	shm_pointer operator--(int) {
		shm_pointer tmp(*this);
		m_offSet -= sizeof(T);
		return tmp;
	}

private:
	int64_t m_offSet;
};

} // namespace smd
