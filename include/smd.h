#pragma once
#include <string>
#include <map>
#include <list>
#include <set>
#include <functional>
#include <cstdarg>
#include <time.h>

//#include "container/string.hpp"
//#include "mem_alloc/alloc.hpp"
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
		: m_owner(owner) {}
	~Env() {}

	//字符串
	void Set(const Slice& key, const Slice& value) {
		auto strKey = key.ToString();
		auto it = m_allStrings.find(strKey);
		if (it == m_allStrings.end()) {
			m_allStrings[strKey] = value.ToString();
		} else {
			it->second = value.ToString();
		}
	}

	bool Get(const Slice& key, Slice* value) const {
		auto strKey = key.ToString();
		auto it = m_allStrings.find(strKey);
		if (it == m_allStrings.end()) {
			return false;
		} else {
			if (value != nullptr) {
				*value = it->second;
			}
			return true;
		}
	}

	bool Del(const Slice& key) {
		auto strKey = key.ToString();
		auto it = m_allStrings.find(strKey);
		if (it == m_allStrings.end()) {
			return false;
		} else {
			it = m_allStrings.erase(it);
			return true;
		}
	}

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
		for (auto it : m_allStrings) {
			const auto& name = it.first;
			const auto& value = it.second;

			Build(*to, STRINGS_NAME, name.data(), name.size());
			Build(*to, STRINGS_VALUE, value.data(), value.size());
		}

		for (auto it : m_allLists) {
			const auto& name = it.first;
			const auto& this_list = it.second;

			Build(*to, LISTS_NAME, name.data(), name.size());
			for (auto& value : this_list) {
				Build(*to, LIST_VALUE, value.data(), value.size());
			}
		}

		for (auto it : m_allMaps) {
			const auto& name = it.first;
			const auto& this_map = it.second;

			Build(*to, MAPS_NAME, name.data(), name.size());
			for (auto it : this_map) {
				const auto& key = it.first;
				const auto& value = it.second;

				Build(*to, MAPS_KEY, key.data(), key.size());
				Build(*to, MAPS_VALUE, value.data(), value.size());
			}
		}

		for (auto it : m_allHashes) {
			const auto& name = it.first;
			const auto& this_hash = it.second;

			Build(*to, HASH_NAME, name.data(), name.size());
			for (auto& value : this_hash) {
				Build(*to, HASH_VALUE, value.data(), value.size());
			}
		}
	}

	void Deserialize(const std::string& from) {
		struct StUnit {
			DataType type;
			size_t size;
			
		};
	}

private:
	EnvMgr& m_owner;
	// Alloc m_alloc;
	Head m_head;

	std::map<std::string, std::string> m_allStrings;
	std::map<std::string, std::list<std::string>> m_allLists;
	std::map<std::string, std::map<std::string, std::string>> m_allMaps;
	std::map<std::string, std::set<std::string>> m_allHashes;
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
