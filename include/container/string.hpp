#pragma once
#include <string>
#include "../common/smd_defines.h"

namespace smd {

template <class _Alloc>
class String : public BaseObj {
public:
	String() {
		m_data = SMD_NULL_PTR;
		m_size = 0;
	}

	String(const std::string& r)
		: m_size(r.size()) {
		m_data = m_alloc.Acuire(m_size);
		memcpy(m_data, r.data(), m_size);
	}

	~String() { clear(); }

	operator=(const std::string& r) {
		clear();
		if (r.size() > 0) {
			m_data = m_alloc.Acuire(r.size());
			memcpy(m_data, r.data(), size);
			m_size = r.size();
		}
	}

	bool empty() { return m_size == 0; }
	size_t size() { return m_size; }
	void clear() {
		if (m_data != SMD_NULL_PTR) {
			m_alloc.Release(m_data);
			m_data = SMD_NULL_PTR;
		}

		m_size = 0;
	}

	const char* data() { return (const char*)m_data; }

	virtual void serialize(std::string& to) override {
		to.append(m_data);
		to.append(m_size);
	}

	virtual void deserialize(const std::string& from, size_t& pos) override {
		assert(from.size() > pos + sizeof(m_data) + sizeof(m_size));
		memcpy(&data, from.data() + pos, sizeof(m_data));
		pos += sizeof(m_data);

		memcpy(&m_size, from.data() + pos, sizeof(m_size));
		pos += sizeof(m_size);
	}

private:
	_Alloc& m_alloc;
	SMD_POINTER m_data;
	size_t m_size;
};
} // namespace smd
