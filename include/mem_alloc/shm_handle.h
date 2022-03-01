#pragma once
#include <cstddef>

#ifdef _WIN32
	#include <mem_alloc/shm_win.h>
#else
	#include "mem_alloc/shm_linux.h"
#endif

namespace smd {

class ShmHandle {
public:
	void* acquire(int shm_key, size_t size, ShareMemOpenMode mode) {
		return m_shm.acquire(shm_key, size, mode);
	}

	void release() {
		m_shm.release();
	}

private:
#ifdef _WIN32
	ShmWin m_shm;
#else
	ShmLinux m_shm;
#endif
};

static ShmHandle g_shmHandle;

} // namespace smd
