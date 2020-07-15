#pragma once
#include <string>
#include <map>
#include "string.hpp"
#include "base_obj.hpp"
#include "../mem_alloc/alloc.hpp"

namespace smd {

template <class T>
class Map : public BaseObj {
public:
	Map(Alloc& alloc, const std::string& name)
		: BaseObj(BaseObj::ObjType::OBJ_MAP)
		, m_alloc(alloc)
		, m_name(alloc, name) {}
	~Map() {}

	std::map<String, T>& GetMap() { return m_map; }

	bool empty() { return m_map.empty(); }
	size_t size() { return m_map.size(); }
	void clear() { m_map.clear(); }

	virtual void serialize(std::string& to) {}
	virtual void deserialize(const char*& buf, size_t& len) {}

private:
	Alloc& m_alloc;
	String m_name;
	std::map<String, T> m_map;
};

} // namespace smd
