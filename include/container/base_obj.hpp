#pragma once
#include <string>
#include "../common/smd_defines.h"

namespace smd {

template <typename T>
inline void ReadStream(T& t, const char*& buf, size_t& len) {
	if (len >= sizeof(t)) {
		memcpy(&t, buf, sizeof(t));
		buf += sizeof(t);
		len -= sizeof(t);
	} else {
		assert(false);
	}
}

class BaseObj {
public:
	enum class ObjType {
		OBJ_BASE_STRING = 1,
		OBJ_STRING,
		OBJ_LIST_NODE,
		OBJ_LIST,
	};

	BaseObj(ObjType type, SMD_POINTER ptr = SMD_NULL_PTR, size_t size = 0)
		: m_ptr(ptr)
		, m_type(type)
		, m_size(size){};

	ObjType GetType() const { return m_type; }
	SMD_POINTER GetPtr() { return m_ptr; }
	size_t GetSize() { return m_size; }

	void SetPtr(SMD_POINTER ptr) { m_ptr = ptr; }
	void SetSize(size_t size) { m_size = size; }

	void Serialize(std::string& to) {
		to.append((const char*)&m_ptr, sizeof(m_ptr));
		to.append((const char*)&m_type, sizeof(m_type));
		to.append((const char*)&m_size, sizeof(m_size));
	}

	void Deserialize(const char*& buf, size_t& len) {
		ReadStream(m_ptr, buf, len);
		ReadStream(m_type, buf, len);
		ReadStream(m_size, buf, len);
	}

protected:
	SMD_POINTER m_ptr; //在共享内存中的地址，可作为唯一id
	ObjType m_type;
	size_t m_size;	//除了上面两个字段外还需要的存储空间
};
} // namespace smd
