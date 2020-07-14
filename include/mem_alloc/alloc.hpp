#pragma once
//#define _USE_SHARE_MEMORY

namespace smd {
class Alloc {
public:
	static void* Malloc(size_t size) {
#ifdef _USE_SHARE_MEMORY

#else
		return malloc(size);
#endif
	}

	static void Free(void* addr) {
#ifdef _USE_SHARE_MEMORY
#else
		free(addr);
#endif
	}

private:
};

} // namespace smd
