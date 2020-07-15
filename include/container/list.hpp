#pragma once
#include <string>
#include <list>
#include "string.hpp"
#include "base_obj.hpp"

namespace smd {

class List : public BaseObj {
public:
	List(Alloc& alloc, const std::string& name)
		: BaseObj(BaseObj::ObjType::OBJ_LIST)
		, m_alloc(alloc)
		, m_name(alloc, name) {}
	~List() {}

	std::list<String>& GetList() { return m_list; }

	bool empty() { return m_list.empty(); }
	size_t size() { return m_list.size(); }
	void clear() { m_list.clear(); }

	virtual void serialize(std::string& to) {}
	virtual void deserialize(const char*& buf, size_t& len) {}

private:
	Alloc& m_alloc;
	String m_name;
	std::list<String> m_list;
};

} // namespace smd
