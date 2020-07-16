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

	virtual void serialize(std::string& to) final {
		m_name.serialize(to);
		size_t size = m_map.size();
		to.append((const char*)&size, sizeof(size));
		for (auto it : m_map) {
			String key = it.first;
			T& value = it.second;
			key.serialize(to);
			value.serialize(to);
		}
	}

	virtual void deserialize(const char*& buf, size_t& len) final { m_name.deserialize(buf, len);
		size_t size = 0;
		ReadStream(size, buf, len);
		for (int i = 0; i < size; i++) {
			String key(m_alloc);
			key.deserialize(buf, len);

			T t(m_alloc);
			t.deserialize(buf, len);

			m_map.insert(std::make_pair(key, t));
		}
	}

private:
	Alloc& m_alloc;
	String m_name;
	std::map<String, T> m_map;
};

} // namespace smd
