#pragma once
#include "../mem_alloc/alloc.h"

namespace smd {

class ShmObj {
public:
	ShmObj(Alloc& alloc)
		: m_alloc(alloc) {}

	ShmObj& operator=(const ShmObj& r) { return *this; }
	Alloc& GetAlloc() const { return m_alloc; }

protected:
	Alloc& m_alloc;
};
} // namespace smd
