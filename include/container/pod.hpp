#pragma once
#include <string>
#include "../common/smd_defines.h"

namespace smd {

template <class _Elem, class _Alloc>
class Pod {
public:
	Pod() { m_data = m_alloc.Acuire(sizeof(_Elem)); }
	~Pod() { m_alloc.Release(m_data); }

	operator=(const _Elem& r) { memcpy(m_data, &r, sizeof(r)); }

private:
	_Alloc& m_alloc;
	SMD_POINTER m_data;
};
} // namespace smd
