#pragma once
#include <string>
#include "../common/smd_defines.h"
#include "pod.hpp"

namespace smd {

template <class _Alloc>
class String {
public:
	String(const std::string& name)
		: m_name(name)
		, m_nodeNum(0) {}
	~String() {}

	operator=(const std::string& r) {
		clear();
		if (r.size() > 0) {
			m_data = m_alloc.Acuire(r.size());
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

private:
	_Alloc& m_alloc;
	Pod<SMD_POINTER, _Alloc> m_data;
	Pod<size_t, _Alloc> m_size;
};
} // namespace smd
