#pragma once
#include "../mem_alloc/alloc.h"

namespace smd {

class ShmObj {
public:
	ShmObj()
		: m_alloc(nullptr) {}

	// 真正的构造函数
	void Construct(Alloc* alloc) { m_alloc = alloc; }

protected:
	Alloc* m_alloc;
};
} // namespace smd
