#pragma once
#include <string>
#include <list>
#include "shm_string.h"
#include "shm_obj.h"
#include "../mem_alloc/alloc.h"

namespace smd {

template <class T>
class ShmList : public ShmObj {
public:
	ShmList(Alloc& alloc, const std::string& name = "")
		: ShmObj(ShmObj::ObjType::OBJ_LIST)
		, m_alloc(alloc)
		, m_name(alloc, name) {}

	//
	// 注意，析构函数里面不能调用clear()，需要使用者主动调用来回收共享内存
	//
	~ShmList() {}

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
	ShmString m_name;
	std::list<T> m_list;
};

} // namespace smd
