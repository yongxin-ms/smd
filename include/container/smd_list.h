#pragma once
#include <string>
#include "smd_defines.h"

namespace smd {
class SmdListNode {
public:
	SmdListNode(const std::string& value)
		: m_data(0)
		, m_len(0)
		, m_next(0)
		, m_prev(0) {}

	bool IsEnd() { return m_next == 0; }

private:
	SMD_POINTER m_data;
	size_t m_len;

	SMD_POINTER m_next;
	SMD_POINTER m_prev;
};

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
	const std::string m_name;
	size_t m_nodeNum;
	SMD_POINTER m_head;
};
} // namespace smd
