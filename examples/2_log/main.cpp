#include <stdio.h>
#include <smd.h>

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

	// 缺省是冷启动，加入参数1表示热启动
	const bool enable_attach = argc == 2 && atoi(argv[1]) == 1;
	auto env = smd::Env::Create(0x001187ca, 25, enable_attach);
	if (env == nullptr) {
		SMD_LOG_ERROR("not enough memory");
		return 0;
	}

	auto& all_lists = env->GetAllLists();
	auto key = smd::shm_string("app_log");
	auto it = all_lists.find(key);
	if (it == all_lists.end()) {
		all_lists.insert(std::make_pair(key, smd::shm_list<smd::shm_string>()));
		it = all_lists.find(key);
	}
	auto& app_log = it->second;

	// 往日志库里面写入一条日志
	app_log.push_back("app started at: " + smd::util::Time::FormatDateTime(time(nullptr)));

	// 从日志库中读出所有日志
	int i = 0;
	for (auto itr = app_log.begin(); itr != app_log.end(); ++itr, ++i) {
		auto& s = *itr;
		printf("[%d] %s\n", i, s.ToString().data());
	}

#ifdef _WIN32
	system("PAUSE");
#endif
	return 0;
}
