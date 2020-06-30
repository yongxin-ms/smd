namespace smd {
class Session {
public:
	//字符串
	void Set(const std::string& key, const std::string& value) {}
	std::string Get(const std::string& key) {}
	bool Del(const std::string& key) { return true; }
	bool Exists(const std::string& key) { return true; }

private:
};

static std::string CreateGuid() { return ""; }
static Session* CreateSession(const std::string& guid) { return nullptr; }

} // namespace smd
