#pragma once
#include <string>
#include <set>
#include "shm_obj.h"
#include "shm_string.h"
#include "../mem_alloc/alloc.h"

namespace smd {

template <class T>
class ShmHash : public ShmObj {
public:
	ShmHash(Alloc& alloc, const std::string& name = "")
		: ShmObj(ShmObj::ObjType::OBJ_HASH)
		, m_alloc(alloc)
		, m_name(alloc, name) {}

	//
	// 注意，析构函数里面不能调用clear()，需要使用者主动调用来回收共享内存
	//
	~ShmHash() {}

	std::set<T>& GetSet() { return m_set; }

	bool empty() { return m_set.empty(); }
	size_t size() { return m_set.size(); }
	void clear() { m_set.clear(); }

private:
	Alloc& m_alloc;
	ShmString m_name;
	std::set<T> m_set;
};

} // namespace smd
