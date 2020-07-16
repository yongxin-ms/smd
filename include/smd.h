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

namespace smd {

class EnvMgr;

class Env {
public:
	Env(EnvMgr& owner)
		: m_owner(owner)
		, m_allStrings(m_alloc)
		, m_allLists(m_alloc)
		, m_allMaps(m_alloc)
		, m_allHashes(m_alloc) {}
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
		}
	}

	bool SGet(const Slice& key, Slice* value) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings.GetMap().find(strKey);
		if (it == m_allStrings.GetMap().end()) {
			return false;
		} else {
			if (value != nullptr) {
				//*value = it->second;
			}
			return true;
		}
	}

	bool SDel(const Slice& key) {
		ShmString strKey(m_alloc, key.ToString());
		auto it = m_allStrings.GetMap().find(strKey);
		if (it == m_allStrings.GetMap().end()) {
			return false;
		} else {
			it = m_allStrings.GetMap().erase(it);
			return true;
		}
	}

	// 还需要一个遍历接口

private:

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
	EnvMgr& m_owner;
	Alloc m_alloc;
	ShmHead m_head;

	ShmMap<ShmString> m_allStrings;
	ShmMap<ShmList<ShmString>> m_allLists;
	ShmMap<ShmMap<ShmString>> m_allMaps;
	ShmMap<ShmHash<ShmString>> m_allHashes;
};

class EnvMgr {
public:
	void SetLogHandler(std::function<void(Log::LogLevel, const char*)> f) {
		m_log.SetLogHandler(f);
	}
	void SetLogLevel(Log::LogLevel lv) { m_log.SetLogLevel(lv); }
	std::string NewGuid() { return ""; }

	enum CreateOptions {
		OPEN_EXIST_ONLY = 1, //只打开不创建
		CREATE_ALWAYS,		 //总是创建
		CREATE_IF_NOT_EXIST, //如果存在打开，否则创建
	};

	Env* CreateEnv(const std::string& guid, CreateOptions option) {
		const char* buf = nullptr;
		ShmHead* head = (ShmHead*)buf;
		if (option == OPEN_EXIST_ONLY) {
			if (head->magic_num != MAGIC_NUM) {
				return nullptr;
			}

			if (strcmp(head->guid, guid.data()) != 0) {
				return nullptr;
			}

		} else if (option == CREATE_ALWAYS) {
			memset(head, 0, sizeof(ShmHead));
			strncpy(head->guid, guid.data(), sizeof(head->guid) - 1);
			head->create_time = time(nullptr);
			++head->visit_num;
			head->magic_num = MAGIC_NUM;
			memset(head->reserve, 0, sizeof(head->reserve));
			head->len = 0;
		} else if (option == CREATE_IF_NOT_EXIST) {

		} else {
			return nullptr;
		}

		return nullptr;
	}

private:
	Log m_log;
};

} // namespace smd
