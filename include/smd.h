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
	Env(void* ptr, unsigned level, bool create_new);
	~Env() {}

	size_t GetUsed() { return m_alloc.GetUsed(); }

	//
	// 内置string, list, map, hash 四种基本数据类型
	//
	shm_map<shm_string, shm_string>& GetAllStrings() { return *m_allStrings; }
	shm_map<shm_string, shm_list<shm_string>>& GetAllLists() { return *m_allLists; }
	shm_map<shm_string, shm_map<shm_string, shm_string>>& GetAllMaps() { return *m_allMaps; }
	shm_map<shm_string, shm_hash<shm_string>>& GetAllHashes() { return *m_allHashes; }

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
	ShmHead& m_head;
	Alloc m_alloc;

	shm_pointer<shm_map<shm_string, shm_string>>& m_allStrings;
	shm_pointer<shm_map<shm_string, shm_list<shm_string>>>& m_allLists;
	shm_pointer<shm_map<shm_string, shm_map<shm_string, shm_string>>>& m_allMaps;
	shm_pointer<shm_map<shm_string, shm_hash<shm_string>>>& m_allHashes;
};

Env::Env(void* ptr, unsigned level, bool create_new)
	: m_head(*((ShmHead*)ptr))
	, m_alloc(ptr, sizeof(ShmHead), level, create_new)
	, m_allStrings(m_head.global_variable.allStrings)
	, m_allLists(m_head.global_variable.allLists)
	, m_allMaps(m_head.global_variable.allMaps)
	, m_allHashes(m_head.global_variable.allHashes) {
	m_head.visit_num++;
	m_head.last_visit_time = time(nullptr);
	g_alloc = &m_alloc;

	if (create_new) {
		if (m_allStrings != shm_nullptr && m_allStrings != 0) {
			m_alloc.Delete(m_allStrings);
		}

		if (m_allLists != shm_nullptr && m_allLists != 0) {
			m_alloc.Delete(m_allLists);
		}

		if (m_allMaps != shm_nullptr && m_allMaps != 0) {
			m_alloc.Delete(m_allMaps);
		}

		if (m_allHashes != shm_nullptr && m_allHashes != 0) {
			m_alloc.Delete(m_allHashes);
		}

		m_allStrings = m_alloc.New<shm_map<shm_string, shm_string>>();
		m_allLists = m_alloc.New<shm_map<shm_string, shm_list<shm_string>>>();
		m_allMaps = m_alloc.New<shm_map<shm_string, shm_map<shm_string, shm_string>>>();
		m_allHashes = m_alloc.New<shm_map<shm_string, shm_hash<shm_string>>>();
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

class EnvMgr {
public:
	EnvMgr() {
		SetLogHandler(
			[](Log::LogLevel lv, const char* msg) {
				std::string time_now = util::Time::FormatDateTime(std::chrono::system_clock::now());
				switch (lv) {
				case Log::LogLevel::kError:
					printf("%s Error: %s\n", time_now.c_str(), msg);
					break;
				case Log::LogLevel::kWarning:
					printf("%s Warning: %s\n", time_now.c_str(), msg);
					break;
				case Log::LogLevel::kInfo:
					printf("%s Info: %s\n", time_now.c_str(), msg);
					break;
				case Log::LogLevel::kDebug:
					printf("%s Debug: %s\n", time_now.c_str(), msg);
					break;
				default:
					break;
				}
			},
			Log::LogLevel::kDebug);
	};

	Env* CreateEnv(int shm_key, unsigned level, ShareMemOpenMode option);

private:
	ShmHandle m_shmHandle;
};

Env* EnvMgr::CreateEnv(int shm_key, unsigned level, ShareMemOpenMode option) {
	size_t size = sizeof(ShmHead) + SmdBuddyAlloc::get_index_size(level) +
				  SmdBuddyAlloc::get_storage_size(level);
	void* ptr = m_shmHandle.acquire(shm_key, size, option);
	if (ptr == nullptr) {
		SMD_LOG_ERROR("acquire failed, %08x:%llu", shm_key, size);
		return nullptr;
	}

	ShmHead* head = (ShmHead*)ptr;
	bool create_new = false;
	if (option == kOpenExist && head->shm_key == shm_key && head->magic_num == MAGIC_NUM &&
		head->total_size == size) {
		SMD_LOG_INFO("attach existed memory, %08x:%llu", shm_key, size);
	} else {
		create_new = true;
		memset(ptr, 0, sizeof(ShmHead));
		head->shm_key = shm_key;
		head->total_size = size;
		head->create_time = time(nullptr);
		head->visit_num = 0;
		head->magic_num = MAGIC_NUM;

		SMD_LOG_INFO("create new memory, %08x:%llu", shm_key, size);
	}

	auto env = new Env(ptr, level, create_new);
	return env;
}

} // namespace smd
