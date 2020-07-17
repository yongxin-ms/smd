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
	Env(Log& log, void* ptr, size_t size, unsigned level)
		: m_log(log)
		, m_alloc(ptr, SERIAL_SIZE, level)
		, m_ptr(ptr)
		, m_size(size)
		, m_allStrings(m_alloc)
		, m_allLists(m_alloc)
		, m_allMaps(m_alloc)
		, m_allHashes(m_alloc) {
		auto head = GetHead();
		head->visit_num++;

		if (head->len > 0) {
			const char* serial_buf = head->data;
			size_t serial_size = head->len;
			deserialize(serial_buf, serial_size);
		}
	}

	~Env() {}

	//字符串
	void SSet(const Slice& key, const Slice& value) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings.GetMap().find(strKey);
		if (it == m_allStrings.GetMap().end()) {
			ShmString strValue(m_alloc, value.ToString());
			m_allStrings.GetMap().insert(std::make_pair(strKey, strValue));
		} else {
			it->second = value.ToString();
			strKey.clear(true);
		}

		Save();
	}

	bool SGet(const Slice& key, Slice* value) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings.GetMap().find(strKey);
		if (it == m_allStrings.GetMap().end()) {
			strKey.clear(true);
			return false;
		} else {
			if (value != nullptr) {
				const auto& strValue = it->second;
				*value = Slice(strValue.data(), strValue.size());
			}

			strKey.clear(true);
			return true;
		}
	}

	bool SDel(const Slice& key) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings.GetMap().find(strKey);
		if (it == m_allStrings.GetMap().end()) {
			strKey.clear(true);
			return false;
		}

		ShmString& strValue = it->second;
		strValue.clear(true);

		it = m_allStrings.GetMap().erase(it);
		strKey.clear(true);

		Save();
		return true;
	}

	// 还需要一个遍历接口

private:

	ShmHead* GetHead() { return (ShmHead*)m_ptr; }
	void Save() {
		std::string to;
		serialize(to);
		if (to.size() >= SERIAL_SIZE) {
			m_log.DoLog(Log::LogLevel::kError, "serialize too long:%llu", to.size());
			return;
		}

		auto head = GetHead();
		memcpy(head->data, to.data(), to.size());
		head->len = to.size();
	}

	void serialize(std::string& to) {
		m_allStrings.serialize(to);
		m_allLists.serialize(to);
		m_allMaps.serialize(to);
		m_allHashes.serialize(to);
	}

	void deserialize(const char*& buf, size_t& len) {
		m_allStrings.deserialize(buf, len);
		m_allLists.deserialize(buf, len);
		m_allMaps.deserialize(buf, len);
		m_allHashes.deserialize(buf, len);
	}

private:
	Log& m_log;
	Alloc m_alloc;
	void* m_ptr;
	const size_t m_size;

	ShmMap<ShmString> m_allStrings;
	ShmMap<ShmList<ShmString>> m_allLists;
	ShmMap<ShmMap<ShmString>> m_allMaps;
	ShmMap<ShmHash<ShmString>> m_allHashes;
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

		size_t size = sizeof(ShmHead) + SERIAL_SIZE + ((uint64_t)1 << level);
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
			head->len = 0;

			m_log.DoLog(Log::LogLevel::kInfo, "create new memory, %s:%llu", guid.data(), size);
		}

		auto env = new Env(m_log, ptr, sizeFact, level);
		return env;
	}

private:
	Log m_log;
	ShmHandle m_shmHandle;
};

} // namespace smd
