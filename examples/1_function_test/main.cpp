#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <smd.h>

#include "test_pointer.h"
#include "test_string.h"
#include "test_vector.h"
#include "test_list.h"
#include "test_hash.h"
#include "test_map.h"

int main(int argc, char* argv[]) {
	smd::SetLogHandler(
		[](smd::Log::LogLevel lv, const char* msg) {
			std::string time_now = smd::util::Time::FormatDateTime(std::chrono::system_clock::now());
			switch (lv) {
			case smd::Log::LogLevel::kError:
				printf("%s Error: %s\n", time_now.c_str(), msg);
				break;
			case smd::Log::LogLevel::kWarning:
				printf("%s Warning: %s\n", time_now.c_str(), msg);
				break;
			case smd::Log::LogLevel::kInfo:
				printf("%s Info: %s\n", time_now.c_str(), msg);
				break;
			case smd::Log::LogLevel::kDebug:
				printf("%s Debug: %s\n", time_now.c_str(), msg);
				break;
			default:
				break;
			}
		},
		smd::Log::LogLevel::kInfo);

	//缺省是冷启动，加入参数1表示热启动
	const bool enable_attach = argc == 2 && atoi(argv[1]) == 1;
	auto env = smd::Env::Create(0x001187fb, 25, enable_attach);
	if (env == nullptr) {
		SMD_LOG_ERROR("Create env failed");
		return 0;
	}

	std::srand((unsigned int)std::time(nullptr));
	for (int i = 0; i < 2; i++) {
		TestPointer test_pointer;
		TestString test_string;
		TestList test_list;
		TestVector test_vector;
		TestHash test_hash;
		TestMap test_map;
	}

	std::string key("StartCounter");
	smd::Slice value;
	int count = 0;
	if (env->SGet(key, &value)) {
		// 如果已经存在
		count = std::stoi(value.ToString());
		count++;
		env->SSet(key, std::to_string(count));

		SMD_LOG_INFO("%s is %d", key.data(), count);
	} else {
		// 如果不存在
		SMD_LOG_INFO("first time run");

		count = 1;
		env->SSet(key, std::to_string(count));
	}

	auto& all_strings = env->GetAllStrings();

	all_strings.insert(std::make_pair(smd::shm_string("will1"), smd::shm_string("1")));
	all_strings.insert(std::make_pair(smd::shm_string("will2"), smd::shm_string("2")));
	all_strings.insert(std::make_pair(smd::shm_string("will3"), smd::shm_string("3")));
	all_strings.insert(std::make_pair(smd::shm_string("will4"), smd::shm_string("4")));
	all_strings.insert(std::make_pair(smd::shm_string("will5"), smd::shm_string("5")));
	all_strings.insert(std::make_pair(smd::shm_string("will6"), smd::shm_string("6")));

	for (int i = 0; i < 20; i++) {
		std::string key1 = smd::util::Text::Format("Hello%03d%03d", count, i);
		std::string value1 = smd::util::Text::Format("World%03d", i);
		all_strings.insert(std::make_pair(smd::shm_string(key1), smd::shm_string(value1)));
	}

	for (auto it = all_strings.begin(); it != all_strings.end(); ++it) {
		const auto& key = it->first;
		const auto& value = it->second;

		SMD_LOG_INFO("Key:%s Value:%s", key.data(), value.data());
	}

	SMD_LOG_INFO("completed");
#ifdef _WIN32
	system("PAUSE");
#endif
	return 0;
}
