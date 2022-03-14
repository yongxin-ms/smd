#pragma once
#include <time.h>
#include <container/shm_string.h>
#include <container/shm_list.h>
#include <container/shm_vector.h>
#include <container/shm_hash.h>
#include <container/shm_map.h>
#include <common/slice.h>
#include <mem_alloc/shm_handle.h>

namespace smd {

template <typename T>
struct ShmHead {
	size_t total_size;
	time_t create_time;
	time_t last_visit_time;
	uint32_t visit_num;
	int shm_key;
	shm_pointer<T> entry;
};

template <typename T>
class Env {
public:
	static Env<T>* Create(int shm_key, unsigned level, bool enable_attach);

	bool IsAttached() const {
		return m_is_attached;
	}

	T& GetEntry() {
		return *m_head.entry;
	}

private:
	Env<T>(void* ptr, bool is_attached);
	Env<T>(const Env<T>&) = delete;
	Env<T>& operator=(const Env<T>&) = delete;

private:
	const bool m_is_attached;
	ShmHead<T>& m_head;
};

template <typename T>
Env<T>::Env(void* ptr, bool is_attached)
	: m_is_attached(is_attached)
	, m_head(*((ShmHead<T>*)ptr)) {
	m_head.visit_num++;
	m_head.last_visit_time = time(nullptr);
	if (!is_attached) {
		m_head.entry = g_alloc->New<T>();
	}
}

template <typename T>
Env<T>* Env<T>::Create(int shm_key, unsigned level, bool enable_attach) {
	size_t size = sizeof(ShmHead<T>) + SmdBuddyAlloc::get_index_size(level) + SmdBuddyAlloc::get_storage_size(level);
	auto [ptr, is_attached] = g_shmHandle.acquire(shm_key, size, enable_attach);
	if (ptr == nullptr) {
		SMD_LOG_ERROR("acquire failed, key:%d, size:%llu", shm_key, size);
		return nullptr;
	}

	if (enable_attach && !is_attached) {
		SMD_LOG_INFO("Attach failed, not exist?");
	}

	ShmHead<T>* head = (ShmHead<T>*)ptr;
	if (is_attached && head->shm_key != shm_key) {
		SMD_LOG_ERROR("Attach failed, shm_key %d mismatch %d", head->shm_key, shm_key);
		is_attached = false;
	}

	if (is_attached && head->total_size != size) {
		SMD_LOG_ERROR("Attach failed, total_size %llu mismatch %llu", head->total_size, size);
		is_attached = false;
	}

	if (!is_attached) {
		memset(ptr, 0, sizeof(ShmHead<T>));
		head->total_size = size;
		head->create_time = time(nullptr);
		head->visit_num = 0;
		head->shm_key = shm_key;

		SMD_LOG_INFO("New env has been created, key:%d, size:%llu", shm_key, size);
	} else {
		SMD_LOG_INFO("Existed env has been attached, key:%d, size:%llu", shm_key, size);
	}

	CreateAlloc(ptr, sizeof(ShmHead<T>), level, is_attached);
	auto env = new Env<T>(ptr, is_attached);
	return env;
}

} // namespace smd
