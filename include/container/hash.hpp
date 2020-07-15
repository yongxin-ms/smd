#pragma once
#include <string>
#include <set>
#include "base_obj.hpp"
#include "string.hpp"
#include "../mem_alloc/alloc.hpp"

namespace smd {

class Hash : public BaseObj {
public:
	Hash(Alloc& alloc, const std::string& name)
		: BaseObj(BaseObj::ObjType::OBJ_HASH)
		, m_alloc(alloc)
		, m_name(alloc, name) {}
	~Hash() {}

	std::set<String>& GetSet() { return m_set; }

	bool empty() { return m_set.empty(); }
	size_t size() { return m_set.size(); }
	void clear() { m_set.clear(); }

	virtual void serialize(std::string& to) {}
	virtual void deserialize(const char*& buf, size_t& len) {}

private:
	Alloc& m_alloc;
	String m_name;
	std::set<String> m_set;
};

} // namespace smd
