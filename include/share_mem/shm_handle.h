#pragma once
#include <cstddef>

#ifdef _WIN32
#include "shm_win.h"
#else
#include "shm_linux.h"
#endif

namespace smd {

class ShmHandle {
public:
	ShmHandle(Log& log)
		: m_shm(log) {}

	bool acquire(const std::string& name, std::size_t size, ShareMemOpenMode mode) {
		return m_shm.acquire(name, size, mode);
	}

	void* get_mem(std::size_t* size) { return m_shm.get_mem(size); }
	void release() { m_shm.release(); }
	void remove() { m_shm.remove(); }
	void remove(const std::string& name) { m_shm.remove(name); }

private:
#ifdef _WIN32
	ShmWin m_shm;
#else
	ShmLinux m_shm;
#endif
};

} // namespace smd
