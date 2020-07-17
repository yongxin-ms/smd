#include <stdio.h>
#include "smd.h"
#include "util.h"

int main() {
	const std::string GUID("0x1001187fb");
	auto mgr = new smd::EnvMgr;
	mgr->SetLogLevel(smd::Log::LogLevel::kDebug);
	mgr->SetLogHandler([](smd::Log::LogLevel lv, const char* s) {
		std::string time_now = Util::Time::FormatDateTime(time(nullptr));
		switch (lv) {
		case smd::Log::LogLevel::kError:
			printf("%s Error: %s\n", time_now.c_str(), s);
			break;
		case smd::Log::LogLevel::kWarning:
			printf("%s Warning: %s\n", time_now.c_str(), s);
			break;
		case smd::Log::LogLevel::kInfo:
			printf("%s Info: %s\n", time_now.c_str(), s);
			break;
		case smd::Log::LogLevel::kDebug:
			printf("%s Debug: %s\n", time_now.c_str(), s);
			break;
		default:
			break;
		}
	});

	auto env = mgr->CreateEnv(GUID, smd::SHM_MIN_SIZE, smd::create | smd::open);
	assert(env != nullptr);

	std::string key("Alice");
	smd::Slice value;
	assert(!env->SGet(key, nullptr));
	assert(!env->SGet(key, &value));
	assert(!env->SDel(key));

	env->SSet(key, "age 18");
	assert(env->SGet(key, nullptr));
	assert(env->SGet(key, &value));
	assert(value == "age 18");
	assert(env->SDel(key));

	assert(!env->SGet(key, nullptr));
	assert(!env->SGet(key, &value));
	assert(!env->SDel(key));

	return 0;
}
