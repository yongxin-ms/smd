#pragma once
#include <string>
#include <map>
#include "container/string.hpp"

namespace smd {
struct SmdHead {
	char guild[16];
	time_t create_time;
	char reserve[256];
	size_t len;
	char data[1];
};

class SmdSession {
public:
	SmdSession(const std::string& guid)
		: m_guid(guid) {
		const char* buf = nullptr;
		SmdHead* head = (SmdHead*)buf;
		if (strcmp(head->guild, guid.data()) == 0) {
			//已经存在，需要恢复现场
			if (head->len > 0) {}

		} else {
			strcpy(head->guild, guid.data());
			head->create_time = time(nullptr);
			memset(head->reserve, 0, sizeof(head->reserve));
			head->len = 0;
		}
	}
	~SmdSession() {}

	void ResetAll() {}

	//字符串
	void Set(const std::string& key, const std::string& value) {}
	std::pair<bool, std::string> Get(const std::string& key) {}
	bool Del(const std::string& key) { return true; }
	bool Exists(const std::string& key) { return true; }

private:
	void Serialize() {}

	void Deserialize() {}

private:
	const std::string m_guid;
	std::map<std::string, String<>> m_allStrings;

};

static std::string CreateGuid() { return ""; }
static SmdSession* CreateSession(const std::string& guid) {
	auto session = new SmdSession(guid);
	return session;
}

} // namespace smd
