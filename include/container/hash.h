#pragma once
#include <string>
#include <set>
#include "base_obj.h"
#include "string.h"
#include "../mem_alloc/alloc.h"

namespace smd {

template <class T>
class Hash : public BaseObj {
public:
	Hash(Alloc& alloc, const std::string& name = "")
		: BaseObj(BaseObj::ObjType::OBJ_HASH)
		, m_alloc(alloc)
		, m_name(alloc, name) {}
	~Hash() {}

	std::set<T>& GetSet() { return m_set; }

	bool empty() { return m_set.empty(); }
	size_t size() { return m_set.size(); }
	void clear() { m_set.clear(); }

	virtual void serialize(std::string& to) override {
		m_name.serialize(to);
		size_t size = m_set.size();
		to.append((const char*)&size, sizeof(size));
		for (auto t : m_set) {
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
			m_set.emplace(t);
		}
	}

private:
	Alloc& m_alloc;
	String m_name;
	std::set<T> m_set;
};

} // namespace smd
