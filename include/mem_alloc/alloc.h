#pragma once
#include <mem_alloc/buddy.h>
#include <container/shm_pointer.h>
#include <common/log.h>

namespace smd {

class Alloc {
public:
	Alloc(void* ptr, size_t off_set, unsigned level, bool attached) {
		const char* basePtr = (const char*)ptr + off_set;
		m_buddy = (SmdBuddyAlloc::buddy*)basePtr;
		g_storagePtr = basePtr + SmdBuddyAlloc::get_index_size(level);

		if (!attached) {
			m_buddy = SmdBuddyAlloc::buddy_new(basePtr, level);

			//
			// 这样能让以后分配的地址不会为0，也不用回收
			//

			Malloc<char>();
		}
	}

	template <class T>
	shm_pointer<T> Malloc(size_t n = 1) {
		auto size = sizeof(T) * n;
		auto addr = _Malloc(size);
		// SMD_LOG_DEBUG("malloc: 0x%p:(%d)", ptr, size);
		return shm_pointer<T>(addr);
	}

	template <class T>
	void Free(shm_pointer<T>& p, size_t n = 1) {
		assert(p != shm_nullptr && p != 0);
		auto size = sizeof(T) * n;
		// SMD_LOG_DEBUG("free: 0x%p:(%d)", p, size);
		_Free(p.Raw(), size);
		p = shm_nullptr;
	}

	template <class T, typename... P>
	shm_pointer<T> New(P&&... params) {
		auto t = Malloc<T>();
		::new (t.Ptr()) T(std::forward<P>(params)...);
		return t;
	}

	template <class T>
	void Delete(shm_pointer<T>& p) {
		(p.Ptr())->~T();
		Free(p);
	}

	template <class T>
	void Delete(T*& p) {
		p->~T();
		auto ptr = ToShmPointer<T>(p);
		Free(ptr);
		p = nullptr;
	}

	size_t GetUsed() const {
		return m_used;
	}

	template <class T>
	shm_pointer<T> ToShmPointer(void* p) const {
		int64_t offset = (const char*)p - g_storagePtr;
		return shm_pointer<T>(offset);
	}

private:
	int64_t _Malloc(size_t size) {
		auto off_set = SmdBuddyAlloc::buddy_alloc(m_buddy, uint32_t(size));
		if (off_set < 0) {
			assert(false);
			return 0;
		}

		SMD_LOG_DEBUG("malloc: 0x%08x:(%llu)", off_set, size);
		m_used += size;
		return off_set;
	}

	void _Free(int64_t off_set, size_t size) {
		SMD_LOG_DEBUG("free: 0x%08x:(%llu)", off_set, size);
		m_used -= size;
		SmdBuddyAlloc::buddy_free(m_buddy, int(off_set));
	}

private:
	SmdBuddyAlloc::buddy* m_buddy;
	size_t m_used = 0;
};

static Alloc* g_alloc = nullptr;

static void CreateAlloc(void* ptr, size_t off_set, unsigned level, bool attached) {
	if (g_alloc != nullptr) {
		delete g_alloc;
		g_alloc = nullptr;
	}

	g_alloc = new Alloc(ptr, off_set, level, attached);
}

} // namespace smd
