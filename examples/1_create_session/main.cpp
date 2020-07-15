﻿#include <stdio.h>
#include "smd.h"
#include "util.h"

int main() {
	const auto GUID("0x1001187fb");
	auto mgr = new smd::EnvMgr;
	mgr->SetLogLevel(smd::EnvMgr::kDebug);
	mgr->SetLogHandler([](smd::EnvMgr::LogLevel lv, const char* s) {
		std::string time_now = Util::Time::FormatDateTime(time(nullptr));
		switch (lv) {
		case smd::EnvMgr::kError:
			printf("%s Error: %s\n", time_now.c_str(), s);
			break;
		case smd::EnvMgr::kWarning:
			printf("%s Warning: %s\n", time_now.c_str(), s);
			break;
		case smd::EnvMgr::kInfo:
			printf("%s Info: %s\n", time_now.c_str(), s);
			break;
		case smd::EnvMgr::kDebug:
			printf("%s Debug: %s\n", time_now.c_str(), s);
			break;
		default:
			break;
		}
	});

	auto env = mgr->CreateEnv(GUID, smd::EnvMgr::CREATE_IF_NOT_EXIST);
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
	env->SDel(key);

	assert(!env->SGet(key, nullptr));
	assert(!env->SGet(key, &value));
	assert(!env->SDel(key));

	return 0;
}
