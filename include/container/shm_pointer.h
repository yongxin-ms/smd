#pragma once

namespace smd {

enum : int64_t {
	shm_nullptr = -1,
};

static const char* g_storage_ptr = nullptr;

template <typename T>
class shm_pointer {
public:
	shm_pointer(int64_t addr = shm_nullptr)
		: m_offset(addr) {}
	shm_pointer(const shm_pointer&) = default;
	shm_pointer& operator=(const shm_pointer&) = default;

	T* Ptr() const {
		assert(m_offset != shm_nullptr && m_offset != 0);
		return (T*)(g_storage_ptr + m_offset);
	}

	T* operator->() const {
		return Ptr();
	}

	T& operator*() const {
		return *Ptr();
	}

	int64_t Raw() const {
		return m_offset;
	}

	bool operator==(const shm_pointer& r) const {
		return m_offset == r.m_offset;
	}

	bool operator!=(const shm_pointer& r) const {
		return m_offset != r.m_offset;
	}

	int64_t operator-(const shm_pointer& r) const {
		return (m_offset - r.m_offset) / sizeof(T);
	}

	shm_pointer operator+(int64_t n) const {
		shm_pointer tmp(*this);
		tmp.m_offset += n * sizeof(T);
		return tmp;
	}

	T& operator[](int64_t n) {
		shm_pointer tmp(*this);
		tmp.m_offset += n * sizeof(T);
		return *tmp;
	}

	const T& operator[](int64_t n) const {
		shm_pointer tmp(*this);
		tmp.m_offset += n * sizeof(T);
		return *tmp;
	}

	shm_pointer& operator++() {
		m_offset += sizeof(T);
		return *this;
	}

	shm_pointer operator++(int) {
		shm_pointer tmp(*this);
		m_offset += sizeof(T);
		return tmp;
	}

	shm_pointer& operator--() {
		m_offset -= sizeof(T);
		return *this;
	}

	shm_pointer operator--(int) {
		shm_pointer tmp(*this);
		m_offset -= sizeof(T);
		return tmp;
	}

private:
	int64_t m_offset;
};

} // namespace smd
