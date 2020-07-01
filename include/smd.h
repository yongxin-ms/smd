#include <string>

namespace smd {
class SmdSession {
public:
	SmdSession(const std::string& guid)
		: m_guid(guid) {}
	~SmdSession() {}

	void ResetAll() {}

	//字符串
	void Set(const std::string& key, const std::string& value) {}
	std::pair<bool, std::string> Get(const std::string& key) {}
	bool Del(const std::string& key) { return true; }
	bool Exists(const std::string& key) { return true; }

private:
	const std::string m_guid;
};

static std::string CreateGuid() { return ""; }
static SmdSession* CreateSession(const std::string& guid) {
	auto session = new SmdSession(guid);
	return session;
}

} // namespace smd
