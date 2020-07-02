#pragma once
#include <string>
#include "../common/smd_defines.h"
#include "pod.hpp"

namespace smd {

template <class _Alloc>
class SmdListNode {
public:
	SmdListNode(const std::string& value)
		: m_data(0)
		, m_len(0)
		, m_next(0)
		, m_prev(0) {}

	bool IsEnd() { return m_next == 0; }

private:
	Pod<SMD_POINTER, _Alloc> m_data;
	Pod<size_t, _Alloc> m_len;

	Pod<SMD_POINTER, _Alloc> m_next;
	Pod<SMD_POINTER, _Alloc> m_prev;
};

template <class _Alloc>
class SmdList {
public:
	SmdList(const std::string& name)
		: m_name(name)
		, m_nodeNum(0) {}
	~SmdList() {}

	void append(const std::string& value) {}
	SmdListNode begin();

	bool erase(const std::string& key) { return false; }
	bool empty() { return m_nodeNum == 0; }
	size_t size() { return m_nodeNum; }
	void clear() {}

private:
	_Alloc& m_alloc;
	Pod<size_t, _Alloc> m_nodeNum;
	Pod<SMD_POINTER, _Alloc> m_head;
};
} // namespace smd
