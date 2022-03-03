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
enum {
	GUID_SIZE = 15,
	MAGIC_NUM = 0x12345678,
};

#pragma pack(push, 1)
struct GlobalVariable {
	shm_pointer<shm_map<shm_string, shm_string>> all_strings;
	shm_pointer<shm_map<shm_string, shm_list<shm_string>>> all_lists;
	shm_pointer<shm_map<shm_string, shm_map<shm_string, shm_string>>> all_maps;
	shm_pointer<shm_map<shm_string, shm_hash<shm_string>>> all_hashes;
};

struct ShmHead {
	size_t total_size;
	time_t create_time;
	time_t last_visit_time;
	uint32_t visit_num;
	uint32_t magic_num;
	GlobalVariable global_variable;
};
#pragma pack(pop)

class Env {
public:
	static Env* Create(int shm_key, unsigned level, bool enable_attach);

	bool IsAttached() const {
		return m_is_attached;
	}

	//
	// 内置string, list, map, hash 四种基本数据类型
	//
	shm_map<shm_string, shm_string>& GetAllStrings() {
		return *m_all_strings;
	}

	shm_map<shm_string, shm_list<shm_string>>& GetAllLists() {
		return *m_all_lists;
	}

	shm_map<shm_string, shm_map<shm_string, shm_string>>& GetAllMaps() {
		return *m_all_maps;
	}

	shm_map<shm_string, shm_hash<shm_string>>& GetAllHashes() {
		return *m_all_hashes;
	}

	//
	// 字符串操作
	//
	// 写操作
	void SSet(const Slice& key, const Slice& value);
	// 读操作
	bool SGet(const Slice& key, Slice* value);
	// 删除操作
	bool SDel(const Slice& key);

private:
	Env(void* ptr, bool is_attached);

private:
	const bool m_is_attached;
	ShmHead& m_head;
	shm_pointer<shm_map<shm_string, shm_string>>& m_all_strings;
	shm_pointer<shm_map<shm_string, shm_list<shm_string>>>& m_all_lists;
	shm_pointer<shm_map<shm_string, shm_map<shm_string, shm_string>>>& m_all_maps;
	shm_pointer<shm_map<shm_string, shm_hash<shm_string>>>& m_all_hashes;
};

Env::Env(void* ptr, bool is_attached)
	: m_is_attached(is_attached)
	, m_head(*((ShmHead*)ptr))
	, m_all_strings(m_head.global_variable.all_strings)
	, m_all_lists(m_head.global_variable.all_lists)
	, m_all_maps(m_head.global_variable.all_maps)
	, m_all_hashes(m_head.global_variable.all_hashes) {
	m_head.visit_num++;
	m_head.last_visit_time = time(nullptr);
	if (!is_attached) {
		m_all_strings = g_alloc->New<shm_map<shm_string, shm_string>>();
		m_all_lists = g_alloc->New<shm_map<shm_string, shm_list<shm_string>>>();
		m_all_maps = g_alloc->New<shm_map<shm_string, shm_map<shm_string, shm_string>>>();
		m_all_hashes = g_alloc->New<shm_map<shm_string, shm_hash<shm_string>>>();

		SMD_LOG_INFO("New env created");
	} else {
		SMD_LOG_INFO("Existing Env attached");
	}
}

// 写操作
void Env::SSet(const Slice& key, const Slice& value) {
	shm_string str_key(key.data(), key.size());
	auto it = m_all_strings->find(str_key);
	if (it == m_all_strings->end()) {
		shm_string str_value(value.data(), value.size());
		m_all_strings->insert(std::make_pair(str_key, str_value));
	} else {
		it->second = value.ToString();
	}
}

// 读操作
bool Env::SGet(const Slice& key, Slice* value) {
	shm_string str_value(key.data(), key.size());
	auto it = m_all_strings->find(str_value);
	if (it == m_all_strings->end()) {
		return false;
	} else {
		if (value != nullptr) {
			const auto& str_value = it->second;

			// 返回的是数据指针，调用者自己决定是否需要拷贝
			*value = Slice(str_value.data(), str_value.size());
		}

		return true;
	}
}

// 删除操作
bool Env::SDel(const Slice& key) {
	shm_string str_value(key.data(), key.size());
	auto it = m_all_strings->find(str_value);
	if (it == m_all_strings->end()) {
		return false;
	}

	it = m_all_strings->erase(it);
	return true;
}

Env* Env::Create(int shm_key, unsigned level, bool enable_attach) {
	size_t size = sizeof(ShmHead) + SmdBuddyAlloc::get_index_size(level) + SmdBuddyAlloc::get_storage_size(level);
	auto [ptr, is_attached] = g_shmHandle.acquire(shm_key, size, enable_attach);
	if (ptr == nullptr) {
		SMD_LOG_ERROR("acquire failed, %08x:%llu", shm_key, size);
		return nullptr;
	}

	if (enable_attach && !is_attached) {
		SMD_LOG_INFO("Attach failed");
	}

	ShmHead* head = (ShmHead*)ptr;
	if (!is_attached || head->magic_num != MAGIC_NUM || head->total_size != size) {
		is_attached = false;

		memset(ptr, 0, sizeof(ShmHead));
		head->total_size = size;
		head->create_time = time(nullptr);
		head->visit_num = 0;
		head->magic_num = MAGIC_NUM;

		SMD_LOG_INFO("Create new %08x:%llu", shm_key, size);
	}

	CreateAlloc(ptr, sizeof(ShmHead), level, is_attached);
	auto env = new Env(ptr, is_attached);
	return env;
}

} // namespace smd
