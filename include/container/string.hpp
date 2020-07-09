#pragma once
#include <string>
#include "../common/smd_defines.h"
#include "base_obj.hpp"

namespace smd {

template <class _Alloc>
class String : public BaseObj {
public:
	String()
		: BaseObj(BaseObj::ObjType::OBJ_STRING, SMD_NULL_PTR, 0) {}

	String(const std::string& r)
		: BaseObj(BaseObj::ObjType::OBJ_STRING) {
		auto ptr = _Alloc::Acquire(r.size());
		SetPtr(ptr);
		SetSize(r.size());
		memcpy(ptr, r.data(), GetSize());
	}

	~String() { clear(); }

	operator=(const std::string& r) {
		clear();
		if (r.size() > 0) {
			auto ptr = _Alloc::Acquire(r.size());
			SetPtr(ptr);
			SetSize(r.size());
			memcpy(ptr, r.data(), GetSize());
		}
	}

	bool empty() { return GetSize() == 0; }
	size_t size() { return GetSize(); }
	void clear() {
		if (GetPtr() != SMD_NULL_PTR) {
			_Alloc::Release(GetPtr());
			SetPtr(SMD_NULL_PTR);
		}

		SetSize(0);
	}

	const char* data() { return (const char*)GetPtr(); }

	void Serialize(std::string& to) { BaseObj::Serialize(to); }
	void Deserialize(const char*& buf, size_t& len) { BaseObj::Deserialize(buf, len); }
};

} // namespace smd
