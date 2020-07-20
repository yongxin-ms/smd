#pragma once
#include <string>
#include "../mem_alloc/alloc.h"
#include "../container/shm_obj.h"

namespace smd {

class ShmString : public ShmObj {
public:
	ShmString(Alloc& alloc, size_t capacity = 0)
		: ShmObj(ShmObj::ObjType::OBJ_STRING)
		, m_alloc(alloc) {
		m_capacity = capacity;
		if (m_capacity > 0) {
			m_ptr = m_alloc.Malloc(m_capacity);
		} else {
			m_ptr = nullptr;
		}

		m_size = 0;
	}

	ShmString(Alloc& alloc, const std::string& r)
		: ShmString(alloc, 0) {
		m_capacity = m_alloc.GetExpectSize(r.size() + 1);
		m_ptr = m_alloc.Malloc(m_capacity);

		memcpy(data(), r.data(), r.size());
		*(data() + r.size()) = '\0';
		m_size = r.size();
	}

	//
	// 注意，析构函数里面不能调用clear()，需要使用者主动调用来回收共享内存
	//
	~ShmString() {}

	ShmString& assign(const std::string& r) {
		if (r.size() < m_capacity) {
			memcpy(data(), r.data(), r.size());
			*(data() + m_size) = '\0';
			m_size = r.size();
			return *this;
		}

		clear(true);

		m_capacity = m_alloc.GetExpectSize(r.size() + 1);
		m_ptr = m_alloc.Malloc(m_capacity);

		memcpy(data(), r.data(), r.size());
		*(data() + r.size()) = '\0';
		m_size = r.size();
		return *this;
	}

	ShmString& operator=(const std::string& r) { return assign(r); }
	int compare(const ShmString& b) const {
		const size_t min_len = (size() < b.size()) ? size() : b.size();
		int r = memcmp(data(), b.data(), min_len);
		if (r == 0) {
			if (size() < b.size())
				r = -1;
			else if (size() > b.size())
				r = +1;
		}
		return r;
	}

	void clear(bool deep = false) {
		if (deep && m_ptr != nullptr) {
			m_alloc.Free(m_ptr, m_capacity);
			m_ptr = nullptr;
			m_capacity = 0;
		}

		m_size = 0;
	}

	std::string ToString() const {
		std::string str(data(), size());
		return str;
	}

private:
	Alloc& m_alloc;
};

inline bool operator==(const ShmString& x, const ShmString& y) {
	return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const ShmString& x, const ShmString& y) { return !(x == y); }

inline bool operator<(const ShmString& x, const ShmString& y) { return x.compare(y) < 0; }

} // namespace smd
