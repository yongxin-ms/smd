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

class Env {
public:
	static Env* Create(int shm_key, unsigned level, bool enable_attach);
	~Env() {}
	bool GetAttached() const {
		return m_attached;
	}

	//
	// 内置string, list, map, hash 四种基本数据类型
	//
	shm_map<shm_string, shm_string>& GetAllStrings() {
		return *m_allStrings;
	}

	shm_map<shm_string, shm_list<shm_string>>& GetAllLists() {
		return *m_allLists;
	}

	shm_map<shm_string, shm_map<shm_string, shm_string>>& GetAllMaps() {
		return *m_allMaps;
	}

	shm_map<shm_string, shm_hash<shm_string>>& GetAllHashes() {
		return *m_allHashes;
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
	Env(void* ptr, bool create_new);

private:
	const bool m_attached;
	ShmHead& m_head;
	shm_pointer<shm_map<shm_string, shm_string>>& m_allStrings;
	shm_pointer<shm_map<shm_string, shm_list<shm_string>>>& m_allLists;
	shm_pointer<shm_map<shm_string, shm_map<shm_string, shm_string>>>& m_allMaps;
	shm_pointer<shm_map<shm_string, shm_hash<shm_string>>>& m_allHashes;
};

Env::Env(void* ptr, bool attached)
	: m_attached(attached)
	, m_head(*((ShmHead*)ptr))
	, m_allStrings(m_head.global_variable.allStrings)
	, m_allLists(m_head.global_variable.allLists)
	, m_allMaps(m_head.global_variable.allMaps)
	, m_allHashes(m_head.global_variable.allHashes) {
	m_head.visit_num++;
	m_head.last_visit_time = time(nullptr);
	if (!attached) {
		if (m_allStrings != shm_nullptr && m_allStrings != 0) {
			SMD_LOG_INFO("clear m_allStrings");
			g_alloc->Delete(m_allStrings);
		}

		if (m_allLists != shm_nullptr && m_allLists != 0) {
			SMD_LOG_INFO("clear m_allLists");
			g_alloc->Delete(m_allLists);
		}

		if (m_allMaps != shm_nullptr && m_allMaps != 0) {
			SMD_LOG_INFO("clear m_allMaps");
			g_alloc->Delete(m_allMaps);
		}

		if (m_allHashes != shm_nullptr && m_allHashes != 0) {
			SMD_LOG_INFO("clear m_allHashes");
			g_alloc->Delete(m_allHashes);
		}

		m_allStrings = g_alloc->New<shm_map<shm_string, shm_string>>();
		m_allLists = g_alloc->New<shm_map<shm_string, shm_list<shm_string>>>();
		m_allMaps = g_alloc->New<shm_map<shm_string, shm_map<shm_string, shm_string>>>();
		m_allHashes = g_alloc->New<shm_map<shm_string, shm_hash<shm_string>>>();

		SMD_LOG_INFO("New env created");
	} else {
		SMD_LOG_INFO("Env attached");
	}
}

// 写操作
void Env::SSet(const Slice& key, const Slice& value) {
	shm_string strKey(key.data(), key.size());
	auto it = m_allStrings->find(strKey);
	if (it == m_allStrings->end()) {
		shm_string strValue(value.data(), value.size());
		m_allStrings->insert(std::make_pair(strKey, strValue));
	} else {
		it->second = value.ToString();
	}
}

// 读操作
bool Env::SGet(const Slice& key, Slice* value) {
	shm_string strKey(key.data(), key.size());
	auto it = m_allStrings->find(strKey);
	if (it == m_allStrings->end()) {
		return false;
	} else {
		if (value != nullptr) {
			const auto& strValue = it->second;

			// 返回的是数据指针，调用者自己决定是否需要拷贝
			*value = Slice(strValue.data(), strValue.size());
		}

		return true;
	}
}

// 删除操作
bool Env::SDel(const Slice& key) {
	shm_string strKey(key.data(), key.size());
	auto it = m_allStrings->find(strKey);
	if (it == m_allStrings->end()) {
		return false;
	}

	it = m_allStrings->erase(it);
	return true;
}

Env* Env::Create(int shm_key, unsigned level, bool enable_attach) {
	size_t size = sizeof(ShmHead) + SmdBuddyAlloc::get_index_size(level) + SmdBuddyAlloc::get_storage_size(level);
	auto [ptr, attached] = g_shmHandle.acquire(shm_key, size, enable_attach);
	if (ptr == nullptr) {
		SMD_LOG_ERROR("acquire failed, %08x:%llu", shm_key, size);
		return nullptr;
	}

	ShmHead* head = (ShmHead*)ptr;
	if (attached && head->magic_num == MAGIC_NUM &&
		head->total_size == size) {
		SMD_LOG_INFO("attach existed memory, %08x:%llu", shm_key, size);
	} else {
		attached = false;
		memset(ptr, 0, sizeof(ShmHead));
		head->total_size = size;
		head->create_time = time(nullptr);
		head->visit_num = 0;
		head->magic_num = MAGIC_NUM;

		SMD_LOG_INFO("create new memory, %08x:%llu", shm_key, size);
	}

	if (g_alloc != nullptr) {
		delete g_alloc;
		g_alloc = nullptr;
	}

	CreateAlloc(ptr, sizeof(ShmHead), level, attached);
	auto env = new Env(ptr, attached);
	return env;
}

} // namespace smd
