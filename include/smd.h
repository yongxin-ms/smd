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
	Env(Log& log, void* ptr, unsigned level, bool create_new)
		: m_log(log)
		, m_alloc(log, ptr, sizeof(ShmHead))
		, m_head(*((ShmHead*)ptr))
		, m_allStrings(m_head.allStrings)
		, m_allLists(m_head.allLists)
		, m_allMaps(m_head.allMaps)
		, m_allHashes(m_head.allHashes) {
		m_head.visit_num++;

		if (create_new) {
			m_alloc.CreateNew(level);
// 			m_allStrings = m_alloc.New<ShmMap<ShmString>>(m_alloc);
// 			m_allLists	 = m_alloc.New<ShmMap<ShmList<ShmString>>>(m_alloc);
// 			m_allMaps	 = m_alloc.New<ShmMap<ShmMap<ShmString>>>(m_alloc);
// 			m_allHashes	 = m_alloc.New<ShmMap<ShmHash<ShmString>>>(m_alloc);
		}
	}

	~Env() {}

	size_t GetUsed() { return m_alloc.GetUsed(); }

	//字符串
	void SSet(const Slice& key, const Slice& value) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			ShmString strValue(m_alloc, value.ToString());
			m_allStrings->insert(make_pair(strKey, strValue));
		} else {
			it->second = value.ToString();
		}
	}

	bool SGet(const Slice& key, Slice* value) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			return false;
		} else {
			if (value != nullptr) {
				const auto& strValue = it->second;
				*value = Slice(strValue.data(), strValue.size());
			}

			return true;
		}
	}

	bool SDel(const Slice& key) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings->find(strKey);
		if (it == m_allStrings->end()) {
			return false;
		}

		ShmString& strValue = it->second;
		it = m_allStrings->erase(it);
		return true;
	}

	// 还需要一个遍历接口

	Alloc& GetMalloc() { return m_alloc; }

private:
	Log& m_log;
	Alloc m_alloc;
	ShmHead& m_head;

	ShmMap<ShmString>*& m_allStrings;
	ShmMap<ShmList<ShmString>>*& m_allLists;
	ShmMap<ShmMap<ShmString>>*& m_allMaps;
	ShmMap<ShmHash<ShmString>>*& m_allHashes;
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
		bool	 create_new = false;
		if (option == open && strcmp(head->guid, guid.data()) == 0 &&
			head->magic_num == MAGIC_NUM && head->total_size == sizeFact) {
			m_log.DoLog(Log::LogLevel::kInfo, "attach existed memory, %s:%llu", guid.data(), size);
		} else {
			create_new = true;
			memset(head, 0, sizeof(ShmHead));
			strncpy(head->guid, guid.data(), sizeof(head->guid) - 1);
			head->total_size  = sizeFact;
			head->create_time = time(nullptr);
			head->visit_num	  = 0;
			head->magic_num	  = MAGIC_NUM;

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
