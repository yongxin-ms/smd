#pragma once
#include <time.h>
#include "container/shm_string.h"
#include "container/shm_list.h"
#include "container/shm_vector.h"
#include "container/shm_hash.h"
#include "container/shm_map.h"
#include "common/slice.h"
#include "share_mem/shm_handle.h"

namespace smd {

class EnvMgr;

class Env {
public:
	Env(Log& log, void* ptr, unsigned level, bool create_new)
		: m_log(log)
		, m_alloc(log, ptr, sizeof(ShmHead), level, create_new)
		, m_head(*((ShmHead*)ptr))
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

			m_allStrings = m_alloc.New<ShmMap<ShmString, ShmString>>();
			m_allLists = m_alloc.New<ShmMap<ShmString, ShmList<ShmString>>>();
			m_allMaps = m_alloc.New<ShmMap<ShmString, ShmMap<ShmString, ShmString>>>();
			m_allHashes = m_alloc.New<ShmMap<ShmString, ShmHash<ShmString>>>();
		}
	}

	~Env() {}

	size_t GetUsed() { return m_alloc.GetUsed(); }

	// 写操作
	void SSet(const Slice& key, const Slice& value) {
		ShmString strKey(key.data(), key.size());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			ShmString strValue(value.data(), value.size());
			m_allStrings->insert(std::make_pair(strKey, strValue));
		} else {
			it->second = value.ToString();
		}
	}

	// 读操作
	bool SGet(const Slice& key, Slice* value) {
		ShmString strKey(key.data(), key.size());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			return false;
		} else {
			if (value != nullptr) {
				const auto& strValue = it->second;

				// 返回的是数据指针，调用者自己是否需要拷贝
				*value = Slice(strValue.data(), strValue.size());
			}

			return true;
		}
	}

	// 删除操作
	bool SDel(const Slice& key) {
		ShmString strKey(key.data(), key.size());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			return false;
		}

		it = m_allStrings->erase(it);
		return true;
	}

	Log& GetLog() { return m_log; }

	//
	// 内置string, list, map, hash四种数据类型
	//
	ShmMap<ShmString, ShmString>& GetAllStrings() { return *m_allStrings; }
	ShmMap<ShmString, ShmList<ShmString>>& GetAllLists() { return *m_allLists; }
	ShmMap<ShmString, ShmMap<ShmString, ShmString>>& GetAllMaps() { return *m_allMaps; }
	ShmMap<ShmString, ShmHash<ShmString>>& GetAllHashes() { return *m_allHashes; }

private:
	Log& m_log;
	Alloc m_alloc;
	ShmHead& m_head;

	ShmPointer<ShmMap<ShmString, ShmString>>& m_allStrings;
	ShmPointer<ShmMap<ShmString, ShmList<ShmString>>>& m_allLists;
	ShmPointer<ShmMap<ShmString, ShmMap<ShmString, ShmString>>>& m_allMaps;
	ShmPointer<ShmMap<ShmString, ShmHash<ShmString>>>& m_allHashes;
};

class EnvMgr {
public:
	EnvMgr()
		: m_shmHandle(m_log){};

	void SetLogHandler(std::function<void(Log::LogLevel, const char*)> f) {
		m_log.SetLogHandler(f);
	}
	void SetLogLevel(Log::LogLevel lv) { m_log.SetLogLevel(lv); }
	std::string NewGuid() { return ""; }

	Env* CreateEnv(const std::string& guid, unsigned level, ShareMemOpenMode option) {
		if (guid.size() > GUID_SIZE) {
			m_log.DoLog(Log::LogLevel::kError, "guid too long, %s", guid.data());
			return nullptr;
		}

		size_t size = sizeof(ShmHead) + SmdBuddyAlloc::get_index_size(level) +
					  SmdBuddyAlloc::get_storage_size(level);
		if (!m_shmHandle.acquire(guid, size, option)) {
			m_log.DoLog(Log::LogLevel::kError, "acquire failed, %s:%llu", guid.data(), size);
			return nullptr;
		}

		size_t sizeFact = 0;
		void* ptr = m_shmHandle.get_mem(&sizeFact);
		if (ptr == nullptr) {
			m_log.DoLog(Log::LogLevel::kError, "get_mem failed, %s:%llu", guid.data(), size);
			return nullptr;
		}

		ShmHead* head = (ShmHead*)ptr;
		bool create_new = false;
		if (option == kOpenExist && strcmp(head->guid, guid.data()) == 0 &&
			head->magic_num == MAGIC_NUM && head->total_size == sizeFact) {
			m_log.DoLog(Log::LogLevel::kInfo, "attach existed memory, %s:%llu", guid.data(), size);
		} else {
			create_new = true;
			memset(head, 0, sizeof(ShmHead));
			strncpy(head->guid, guid.data(), sizeof(head->guid) - 1);
			head->total_size = sizeFact;
			head->create_time = time(nullptr);
			head->visit_num = 0;
			head->magic_num = MAGIC_NUM;

			m_log.DoLog(Log::LogLevel::kInfo, "create new memory, %s:%llu", guid.data(), size);
		}

		auto env = new Env(m_log, ptr, level, create_new);
		return env;
	}

private:
	Log m_log;
	ShmHandle m_shmHandle;
};

} // namespace smd
