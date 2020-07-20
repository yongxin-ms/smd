#pragma once
#include <string>
#include "../common/shm_defines.h"

namespace smd {

class ShmObj {
public:
	enum class ObjType {
		OBJ_STRING,
		OBJ_LIST,
		OBJ_HASH,
		OBJ_MAP,
	};

	ShmObj(ObjType type)
		: m_type(type) {
		m_ptr = nullptr;
		m_capacity = 0;
		m_size = 0;
	}

	char* data() { return (char*)m_ptr; }
	const char* data() const { return (const char*)m_ptr; }
	size_t size() const { return m_size; }
	bool empty() { return m_size > 0; }
	size_t capacity() const { return m_capacity; }

protected:
	ObjType m_type;
	void* m_ptr; //在共享内存中的地址，可作为唯一id
	size_t m_capacity;
	size_t m_size;
};
} // namespace smd
