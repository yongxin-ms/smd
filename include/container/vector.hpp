#pragma once
#include <string>
#include "../common/smd_defines.h"

namespace smd {

template <class _Elem, class _Alloc>
class Vector {
public:
	Vector(const std::string& name)
		: m_name(name)
		, m_nodeNum(0) {}
	~Vector() {}

	void append(const std::string& value) {}
	SmdListNode begin();

	bool erase(const std::string& key) { return false; }
	bool empty() { return m_nodeNum == 0; }
	size_t size() { return m_nodeNum; }
	void clear() {}

private:
	const std::string m_name;
	size_t m_nodeNum;
	SMD_POINTER m_head;
};
} // namespace smd
