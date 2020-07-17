#pragma once
#include <string>
#include <map>
#include "shm_string.h"
#include "shm_obj.h"
#include "../mem_alloc/alloc.h"

namespace smd {

template <class T>
class ShmMap : public ShmObj {
public:
	ShmMap(Alloc& alloc, const std::string& name = "")
		: ShmObj(ShmObj::ObjType::OBJ_MAP)
		, m_alloc(alloc)
		, m_name(alloc, name) {}

	//
	// 注意，析构函数里面不能调用clear()，需要使用者主动调用来回收共享内存
	//
	~ShmMap() {}

	std::map<ShmString, T>& GetMap() { return m_map; }

	bool empty() { return m_map.empty(); }
	size_t size() { return m_map.size(); }
	void clear() { m_map.clear(); }

	virtual void serialize(std::string& to) override {
		m_name.serialize(to);
		size_t size = m_map.size();
		to.append((const char*)&size, sizeof(size));
		for (auto it : m_map) {
			ShmString key = it.first;
			T& value = it.second;
			key.serialize(to);
			value.serialize(to);
		}
	}

	virtual void deserialize(const char*& buf, size_t& len) override {
		m_name.deserialize(buf, len);
		size_t size = 0;
		ReadStream(size, buf, len);
		for (int i = 0; i < size; i++) {
			ShmString key(m_alloc);
			key.deserialize(buf, len);

			T t(m_alloc);
			t.deserialize(buf, len);

			m_map.emplace(std::make_pair(key, t));
		}
	}

private:
	Alloc& m_alloc;
	ShmString m_name;
	std::map<ShmString, T> m_map;
};

} // namespace smd
