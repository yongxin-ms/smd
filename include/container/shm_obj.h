#pragma once
#include "../mem_alloc/alloc.h"

namespace smd {

class ShmObj {
public:
	ShmObj(Alloc& alloc)
		: m_alloc(alloc) {}

protected:
	Alloc& m_alloc;
};
} // namespace smd
