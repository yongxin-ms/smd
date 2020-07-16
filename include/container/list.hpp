#pragma once
#include <string>
#include <list>
#include "string.hpp"
#include "base_obj.hpp"
#include "../mem_alloc/alloc.hpp"

namespace smd {

template <class T>
class List : public BaseObj {
public:
	List(Alloc& alloc, const std::string& name = "")
		: BaseObj(BaseObj::ObjType::OBJ_LIST)
		, m_alloc(alloc)
		, m_name(alloc, name) {}
	~List() {}

	std::list<T>& GetList() { return m_list; }

	bool empty() { return m_list.empty(); }
	size_t size() { return m_list.size(); }
	void clear() { m_list.clear(); }

	virtual void serialize(std::string& to) override {
		m_name.serialize(to);
		size_t size = m_list.size();
		to.append((const char*)&size, sizeof(size));
		for (auto& t : m_list) {
			t.serialize(to);
		}
	}

	virtual void deserialize(const char*& buf, size_t& len) override {
		m_name.deserialize(buf, len);
		size_t size = 0;
		ReadStream(size, buf, len);
		for (int i = 0; i < size; i++) {
			T t(m_alloc);
			t.deserialize(buf, len);
			m_list.emplace_back(t);
		}
	}

private:
	Alloc& m_alloc;
	String m_name;
	std::list<T> m_list;
};

} // namespace smd
