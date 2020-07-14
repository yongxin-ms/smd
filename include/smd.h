#pragma once
#include <string>
#include <map>
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

	void Serialize(std::string* to) const {}
	void Deserialize(const std::string& from) {}

private:
	//Alloc m_alloc;
	Head m_head;
	EnvMgr& m_owner;
	std::map<std::string, std::string> m_allStrings;
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
