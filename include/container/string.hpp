#pragma once
#include <string>
#include "../mem_alloc/alloc.hpp"
#include "../container/base_obj.hpp"

namespace smd {

class String : public BaseObj {
public:
	String(Alloc& alloc, size_t capacity)
		: BaseObj(BaseObj::ObjType::OBJ_STRING)
		, m_alloc(alloc) {
		m_capacity = capacity;
		if (m_capacity > 0) {
			m_ptr = m_alloc.Malloc(m_capacity);
		} else {
			m_ptr = SMD_NULL_PTR;
		}

		m_size = 0;
	}

	String(Alloc& alloc, const std::string& r)
		: String(alloc, 0) {
		m_capacity = m_alloc.GetExpectSize(r.size() + 1);
		m_ptr = m_alloc.Malloc(m_capacity);
		
		memcpy(data(), r.data(), r.size());
		*(data() + m_size) = '\0';
		m_size = r.size();
	}

	~String() { clear(true); }

	String& assign(const std::string& r) {
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
		*(data() + m_size) = '\0';
		m_size = r.size();
		return *this;
	}

	String& operator=(const std::string& r) { return assign(r); }
	int compare(const String& b) const {
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
		if (deep && m_ptr != SMD_NULL_PTR) {
			m_alloc.Free(m_ptr);
			m_ptr = SMD_NULL_PTR;
			m_capacity = 0;
		}

		m_size = 0;
	}

	virtual void serialize(std::string& to) {}
	virtual void deserialize(const char*& buf, size_t& len) {}

private:
	Alloc& m_alloc;
};

inline bool operator==(const String& x, const String& y) {
	return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const String& x, const String& y) { return !(x == y); }

inline bool operator<(const String& x, const String& y) { return x.compare(y) < 0; }

} // namespace smd
