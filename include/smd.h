#pragma once
#include <string>
#include <map>
#include <list>
#include <set>
#include <time.h>

#include "container/shm_string.h"
#include "container/shm_list.h"
#include "container/shm_hash.h"
#include "container/shm_map.h"
#include "mem_alloc/alloc.h"
#include "common/slice.h"
#include "common/log.h"
#include "share_mem/shm_handle.h"

namespace smd {

class EnvMgr;

class Env {
public:
	Env(Log& log, void* ptr, unsigned level)
		: m_log(log)
		, m_alloc(ptr, sizeof(ShmHead), level)
		, m_ptr(ptr) {
		auto head = GetHead();
		head->visit_num++;

		m_allStrings = (ShmMap<ShmString*>*)head->global_pointer[GLOBAL_POINTER_ALL_STRINGS];
		m_allLists = (ShmMap<ShmList<ShmString*>*>*)head->global_pointer[GLOBAL_POINTER_ALL_LISTS];
		m_allMaps = (ShmMap<ShmMap<ShmString*>*>*)head->global_pointer[GLOBAL_POINTER_ALL_MAPS];
		m_allHashes =
			(ShmMap<ShmHash<ShmString*>*>*)head->global_pointer[GLOBAL_POINTER_ALL_HASHES];

		if (m_allStrings == nullptr) {
			m_allStrings = m_alloc.New<ShmMap<ShmString*>>(m_alloc);
			memcpy(&head->global_pointer[GLOBAL_POINTER_ALL_STRINGS], m_allStrings,
				sizeof(m_allStrings));
		}

		if (m_allLists == nullptr) {
			m_allLists = m_alloc.New<ShmMap<ShmList<ShmString*>*>>(m_alloc);
			memcpy(&head->global_pointer[GLOBAL_POINTER_ALL_LISTS], m_allLists, sizeof(m_allLists));
		}

		if (m_allMaps == nullptr) {
			m_allMaps = m_alloc.New<ShmMap<ShmMap<ShmString*>*>>(m_alloc);
			memcpy(&head->global_pointer[GLOBAL_POINTER_ALL_MAPS], m_allMaps, sizeof(m_allMaps));
		}

		if (m_allHashes == nullptr) {
			m_allHashes = m_alloc.New<ShmMap<ShmHash<ShmString*>*>>(m_alloc);
			memcpy(
				&head->global_pointer[GLOBAL_POINTER_ALL_HASHES], m_allHashes, sizeof(m_allHashes));
		}
	}

	~Env() {}

	size_t GetUsed() { return m_alloc.GetUsed(); }

	//字符串
	void SSet(const Slice& key, const Slice& value) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			ShmString* strValue = m_alloc.New<ShmString>(m_alloc, value.ToString());
			m_allStrings->insert(make_pair(strKey, strValue));
		} else {
			*it->second = value.ToString();
			strKey.clear(true);
		}
	}

	bool SGet(const Slice& key, Slice* value) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			strKey.clear(true);
			return false;
		} else {
			if (value != nullptr) {
				const auto& strValue = it->second;
				*value = Slice(strValue->data(), strValue->size());
			}

			strKey.clear(true);
			return true;
		}
	}

	bool SDel(const Slice& key) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			strKey.clear(true);
			return false;
		}

		ShmString strOriKey = it->first;
		strOriKey.clear(true);
		ShmString& strValue = *it->second;
		strValue.clear(true);

		it = m_allStrings->erase(it);
		strKey.clear(true);

		return true;
	}

	// 还需要一个遍历接口

private:
	ShmHead* GetHead() { return (ShmHead*)m_ptr; }

private:
	Log& m_log;
	Alloc m_alloc;
	void* m_ptr;

	ShmMap<ShmString*>* m_allStrings;
	ShmMap<ShmList<ShmString*>*>* m_allLists;
	ShmMap<ShmMap<ShmString*>*>* m_allMaps;
	ShmMap<ShmHash<ShmString*>*>* m_allHashes;
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

	Env* CreateEnv(const std::string& guid, unsigned level, unsigned option) {
		if (guid.size() > GUID_SIZE) {
			m_log.DoLog(Log::LogLevel::kError, "guid too long, %s", guid.data());
			return nullptr;
		}

		size_t size = sizeof(ShmHead) + SmdBuddyAlloc::get_need_size(level);
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
		if (strcmp(head->guid, guid.data()) == 0 && head->magic_num == MAGIC_NUM &&
			head->total_size == sizeFact) {
			m_log.DoLog(Log::LogLevel::kInfo, "attach existed memory, %s:%llu", guid.data(), size);
		} else {
			memset(head, 0, sizeof(ShmHead));
			strncpy(head->guid, guid.data(), sizeof(head->guid) - 1);
			head->total_size = sizeFact;
			head->create_time = time(nullptr);
			head->visit_num = 0;
			head->magic_num = MAGIC_NUM;
			memset(head->reserve, 0, sizeof(head->reserve));

			m_log.DoLog(Log::LogLevel::kInfo, "create new memory, %s:%llu", guid.data(), size);
		}

		auto env = new Env(m_log, ptr, level);
		return env;
	}

private:
	Log m_log;
	ShmHandle m_shmHandle;
};

} // namespace smd
