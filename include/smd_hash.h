#pragma once
#include <string>

namespace smd {
class SmdHash {
public:
	SmdHash(const std::string& name, size_t capacity)
		: m_name(name)
		, m_capacity(capacity)
		, m_nodeNum(0) {}
	~SmdHash() {}

	void insert(const std::string& key, const std::string& value) {}
	bool find(const std::string& key, std::string& value) { return false; }
	bool erase(const std::string& key) { return false; }
	bool empty() { return m_nodeNum == 0; }
	size_t size() { return m_nodeNum; }
	void clear() {}

private:
	const std::string m_name;
	size_t m_capacity;
	size_t m_nodeNum;
};
} // namespace smd
