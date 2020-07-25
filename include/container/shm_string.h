#pragma once
#include <string>
#include "shm_obj.h"
#include "../common/utility.h"

namespace smd {

class ShmString : public ShmObj {
public:
	ShmString(Alloc& alloc, size_t capacity = 0)
		: ShmObj(alloc) {
		m_capacity = GetSuitableCapacity(capacity);
		m_ptr	   = m_alloc.Malloc<char>(m_capacity);
		m_size	   = 0;
	}

	ShmString(Alloc& alloc, const std::string& r)
		: ShmObj(alloc) {
		m_capacity = GetSuitableCapacity(r.size() + 1);
		m_ptr	   = m_alloc.Malloc<char>(m_capacity);

		memcpy(data(), r.data(), r.size());
		*(data() + r.size()) = '\0';
		m_size				 = r.size();
	}

	// 真正的构造函数
	ShmString(Alloc& alloc, const ShmString& r)
		: ShmObj(alloc) {
		m_capacity = r.capacity();
		m_ptr	   = m_alloc.Malloc<char>(m_capacity);

		memcpy(data(), r.data(), r.size());
		*(data() + r.size()) = '\0';
		m_size				 = r.size();
	}

	~ShmString() {
		if (m_ptr != nullptr) {
			m_alloc.Free(m_ptr, m_capacity);
			m_capacity = 0;
		}

		m_size = 0;
	}

	char*		data() { return m_ptr; }
	const char* data() const { return (const char*)m_ptr; }
	size_t		size() const { return m_size; }
	bool		empty() { return m_size > 0; }
	size_t		capacity() const { return m_capacity; }

	ShmString& assign(const std::string& r) {
		if (r.size() < m_capacity) {
			memcpy(data(), r.data(), r.size());
			*(data() + r.size()) = '\0';
			m_size				 = r.size();
			return *this;
		}

		if (m_ptr != nullptr) {
			m_alloc.Free(m_ptr, m_capacity);
			m_capacity = 0;
		}

		m_capacity = GetSuitableCapacity(r.size() + 1);
		m_ptr	   = m_alloc.Malloc<char>(m_capacity);

		memcpy(data(), r.data(), r.size());
		*(data() + r.size()) = '\0';
		m_size				 = r.size();
		return *this;
	}

	ShmString& operator=(const std::string& r) { return assign(r); }

	int compare(const ShmString& b) const {
		const size_t min_len = (size() < b.size()) ? size() : b.size();
		int			 r		 = memcmp(data(), b.data(), min_len);
		if (r == 0) {
			if (size() < b.size())
				r = -1;
			else if (size() > b.size())
				r = +1;
		}
		return r;
	}

	void clear() { m_size = 0; }

	std::string ToString() const { return std::string(data(), size()); }

private:
	static size_t GetSuitableCapacity(size_t size) {
		if (size <= 16)
			return 16;
		else
			return Utility::NextPowOf2(size);
	}

private:
	char*  m_ptr;
	size_t m_capacity;
	size_t m_size;
};

inline bool operator==(const ShmString& x, const ShmString& y) {
	return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const ShmString& x, const ShmString& y) { return !(x == y); }
inline bool operator<(const ShmString& x, const ShmString& y) { return x.compare(y) < 0; }
inline bool operator>(const ShmString& x, const ShmString& y) { return x.compare(y) > 0; }

} // namespace smd
