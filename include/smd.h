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
		, m_allStrings(m_alloc, "")
		, m_allLists(m_alloc, "")
		, m_allMaps(m_alloc, "")
		, m_allHashes(m_alloc, "") {}
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

	enum DataType : unsigned char{
		STRINGS_NAME = 1,
		STRINGS_VALUE,

		LISTS_NAME,
		LIST_VALUE,

		MAPS_NAME,
		MAPS_KEY,
		MAPS_VALUE,

		HASH_NAME,
		HASH_VALUE,
	};

	static void Build(std::string& data, DataType type, const void* buf, uint32_t size) {
		data.append((const char*)&type, sizeof(type));
		data.append((const char*)&size, sizeof(size));
		if (size > 0) {
			data.append((const char*)buf, size);
		}
	}

	void Serialize(std::string* to) const {
// 		for (auto it : m_allStrings.GetMap()) {
// 			const auto& name = it.first;
// 			const auto& value = it.second;
// 
// 			Build(*to, STRINGS_NAME, name.data(), name.size());
// 			Build(*to, STRINGS_VALUE, value.data(), value.size());
// 		}
// 
// 		for (auto it : m_allLists) {
// 			const auto& name = it.first;
// 			const auto& this_list = it.second;
// 
// 			Build(*to, LISTS_NAME, name.data(), name.size());
// 			for (auto& value : this_list) {
// 				Build(*to, LIST_VALUE, value.data(), value.size());
// 			}
// 		}
// 
// 		for (auto it : m_allMaps) {
// 			const auto& name = it.first;
// 			const auto& this_map = it.second;
// 
// 			Build(*to, MAPS_NAME, name.data(), name.size());
// 			for (auto it : this_map) {
// 				const auto& key = it.first;
// 				const auto& value = it.second;
// 
// 				Build(*to, MAPS_KEY, key.data(), key.size());
// 				Build(*to, MAPS_VALUE, value.data(), value.size());
// 			}
// 		}
// 
// 		for (auto it : m_allHashes) {
// 			const auto& name = it.first;
// 			const auto& this_hash = it.second;
// 
// 			Build(*to, HASH_NAME, name.data(), name.size());
// 			for (auto& value : this_hash) {
// 				Build(*to, HASH_VALUE, value.data(), value.size());
// 			}
// 		}
	}

	void Deserialize(const std::string& from) {
		struct StUnit {
			DataType type;
			size_t size;
			
		};
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
