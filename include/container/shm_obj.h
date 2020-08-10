#pragma once
#include "../mem_alloc/alloc.h"

namespace smd {

class ShmObj {
public:
	ShmObj(Alloc& alloc)
		: m_alloc(alloc) {}

	ShmObj& operator=(const ShmObj& r) {
		return *this;
	}

protected:
	Alloc& m_alloc;
};
} // namespace smd
