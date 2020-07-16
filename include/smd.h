#pragma once
#include <string>
#include <map>
#include <list>
#include <set>
#include <functional>
#include <cstdarg>
#include <time.h>

#include "container/string.hpp"
#include "container/list.hpp"
#include "container/hash.hpp"
#include "container/map.hpp"
#include "mem_alloc/alloc.hpp"
#include "common/slice.h"

namespace smd {
enum {
	MAGIC_NUM = 0x12345678,
};

struct Head {
	Head();

	char guid[16];
	time_t create_time;
	uint32_t visit_num;
	uint32_t magic_num;
	char reserve[256];
	size_t len;
	char data[1];
};

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
		String strKey(m_alloc, key.ToString());
		auto it = m_allStrings.GetMap().find(strKey);
		if (it == m_allStrings.GetMap().end()) {
			String strValue(m_alloc, value.ToString());
			m_allStrings.GetMap().insert(std::make_pair(strKey, strValue));
		} else {
			it->second = value.ToString();
		}
	}

	bool SGet(const Slice& key, Slice* value) {
		String strKey(m_alloc, key.ToString());
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
		String strKey(m_alloc, key.ToString());
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
	Head m_head;

	Map<String> m_allStrings;
 	Map<List<String>> m_allLists;
 	Map<Map<String>> m_allMaps;
 	Map<Hash<String>> m_allHashes;
};

class EnvMgr {
public:
	enum LogLevel {
		kError = 1,
		kWarning,
		kInfo,
		kDebug,
	};

	void SetLogHandler(std::function<void(LogLevel, const char*)> f) { log_func_ = f; }
	void SetLogLevel(LogLevel lv) { log_level_ = lv; }

	void Log(LogLevel lv, const char* fmt, ...) {
		if (lv <= log_level_ && log_func_) {
			char buf[4096];
			va_list args;
			va_start(args, fmt);
			vsnprintf(buf, sizeof(buf) - 1, fmt, args);
			va_end(args);
			log_func_(lv, buf);
		}
	}

	std::string NewGuid() { return ""; }

	enum CreateOptions {
		OPEN_EXIST_ONLY = 1, //只打开不创建
		CREATE_ALWAYS,		 //总是创建
		CREATE_IF_NOT_EXIST, //如果存在打开，否则创建
	};

	Env* CreateEnv(const std::string& guid, CreateOptions option) {
		const char* buf = nullptr;
		Head* head = (Head*)buf;
		if (option == OPEN_EXIST_ONLY) {
			if (head->magic_num != MAGIC_NUM) {
				return nullptr;
			}

			if (strcmp(head->guid, guid.data()) != 0) {
				return nullptr;
			}

		} else if (option == CREATE_ALWAYS) {
			memset(head, 0, sizeof(Head));
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
	int log_level_ = kDebug;
	std::function<void(LogLevel, const char*)> log_func_ = nullptr;
};

} // namespace smd
