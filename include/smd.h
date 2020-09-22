#pragma once
#include <time.h>
#include "container/shm_string.h"
#include "container/shm_list.h"
#include "container/shm_vector.h"
#include "container/shm_hash.h"
#include "container/shm_map.h"
#include "common/slice.h"
#include "mem_alloc/shm_handle.h"

namespace smd {

class Env {
public:
	Env(Log& log, void* ptr, unsigned level, bool create_new);
	~Env() {}

	size_t GetUsed() { return m_alloc.GetUsed(); }
	Log& GetLog() { return m_log; }

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
	Log& m_log;
	ShmHead& m_head;
	Alloc m_alloc;

	shm_pointer<shm_map<shm_string, shm_string>>& m_allStrings;
	shm_pointer<shm_map<shm_string, shm_list<shm_string>>>& m_allLists;
	shm_pointer<shm_map<shm_string, shm_map<shm_string, shm_string>>>& m_allMaps;
	shm_pointer<shm_map<shm_string, shm_hash<shm_string>>>& m_allHashes;
};

Env::Env(Log& log, void* ptr, unsigned level, bool create_new)
	: m_log(log)
	, m_head(*((ShmHead*)ptr))
	, m_alloc(log, ptr, sizeof(ShmHead), level, create_new)
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
	EnvMgr()
		: m_shmHandle(m_log){};

	void SetLogHandler(std::function<void(Log::LogLevel, const char*)> f) {
		m_log.SetLogHandler(f);
	}
	void SetLogLevel(Log::LogLevel lv) { m_log.SetLogLevel(lv); }
	std::string NewGuid() { return ""; }
	Env* CreateEnv(const std::string& guid, unsigned level, ShareMemOpenMode option);

private:
	Log m_log;
	ShmHandle m_shmHandle;
};

Env* EnvMgr::CreateEnv(const std::string& guid, unsigned level, ShareMemOpenMode option) {
	if (guid.size() > GUID_SIZE) {
		m_log.DoLog(Log::LogLevel::kError, "guid too long, %s", guid.data());
		return nullptr;
	}

	size_t size = sizeof(ShmHead) + SmdBuddyAlloc::get_index_size(level) +
				  SmdBuddyAlloc::get_storage_size(level);
	void* ptr = m_shmHandle.acquire(guid, size, option);
	if (ptr == nullptr) {
		m_log.DoLog(Log::LogLevel::kError, "acquire failed, %s:%llu", guid.data(), size);
		return nullptr;
	}

	ShmHead* head = (ShmHead*)ptr;
	bool create_new = false;
	if (option == kOpenExist && strcmp(head->guid, guid.data()) == 0 &&
		head->magic_num == MAGIC_NUM && head->total_size == size) {
		m_log.DoLog(Log::LogLevel::kInfo, "attach existed memory, %s:%llu", guid.data(), size);
	} else {
		create_new = true;
		memset(head, 0, sizeof(ShmHead));
		strncpy(head->guid, guid.data(), sizeof(head->guid) - 1);
		head->total_size = size;
		head->create_time = time(nullptr);
		head->visit_num = 0;
		head->magic_num = MAGIC_NUM;

		m_log.DoLog(Log::LogLevel::kInfo, "create new memory, %s:%llu", guid.data(), size);
	}

	auto env = new Env(m_log, ptr, level, create_new);
	return env;
}

} // namespace smd
