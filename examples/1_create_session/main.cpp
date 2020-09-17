#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "smd.h"
#include "util.h"

#include "test_pointer.h"
#include "test_string.h"
#include "test_vector.h"
#include "test_list.h"
#include "test_hash.h"
#include "test_map.h"

int main(int argc, char* argv[]) {

	//缺省是冷启动，加入参数1表示热启动
	smd::ShareMemOpenMode openMode = smd::kCreateAlways;
	if (argc == 2 && atoi(argv[1]) == 1) {
		openMode = smd::kOpenExist;
	}

	auto mgr = new smd::EnvMgr;
	mgr->SetLogLevel(smd::Log::LogLevel::kInfo);
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

	const std::string GUID("0x1001187fb");
	auto env = mgr->CreateEnv(GUID, 25, openMode);
	assert(env != nullptr);
	auto& log = env->GetLog();

	std::srand((unsigned int)std::time(nullptr));
// 	for (int i = 0; i < 10; i++) {
// 		TestPointer test_pointer(log);
// 		TestString test_string(log);
// 		TestList test_list(log);
// 		TestVector test_vector(log);
// 		TestHash test_hash(log);
// 		TestMap test_map(log);
// 	}

	std::string key("StartCounter");
	smd::Slice value;
	int count = 0;
	if (env->SGet(key, &value)) {
		// 如果已经存在
		count = std::stoi(value.ToString());
		count++;
		env->SSet(key, std::to_string(count));

		log.DoLog(smd::Log::LogLevel::kInfo, "%s is %d", key.data(), count);
	} else {
		// 如果不存在
		log.DoLog(smd::Log::LogLevel::kInfo, "first time run");

		count = 1;
		env->SSet(key, std::to_string(count));
	}

	auto& all_strings = env->GetAllStrings();
	//auto& all_strings = *smd::g_alloc->New<smd::shm_map<smd::shm_string, smd::shm_string>>();

	all_strings.insert(std::make_pair(smd::shm_string("will1"), smd::shm_string("1")));
	all_strings.insert(std::make_pair(smd::shm_string("will2"), smd::shm_string("2")));
	all_strings.insert(std::make_pair(smd::shm_string("will3"), smd::shm_string("3")));
	all_strings.insert(std::make_pair(smd::shm_string("will4"), smd::shm_string("4")));
	all_strings.insert(std::make_pair(smd::shm_string("will5"), smd::shm_string("5")));
	all_strings.insert(std::make_pair(smd::shm_string("will6"), smd::shm_string("6")));

	for (int i = 0; i < 20; i++) {
		std::string key1 = Util::Text::Format("Hello%03d%03d", count, i);
		std::string value1 = Util::Text::Format("World%03d", i);
		all_strings.insert(std::make_pair(smd::shm_string(key1), smd::shm_string(value1)));
	}

	for (auto it = all_strings.begin(); it != all_strings.end(); ++it) {
		const auto& key = it->first;
		const auto& value = it->second;

		log.DoLog(smd::Log::LogLevel::kInfo, "Key:%s Value:%s", key.data(), value.data());
	}

#ifdef _WIN32
	int n = 0;
	std::cin >> n;
#endif
	return 0;
}
