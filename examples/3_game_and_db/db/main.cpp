#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "sm_env.h"

#include "main_db.h"
#include "DataCenter.h"
#include "../server_common/server_common_def.h"

int main(int argc, char* argv[]) {
	int ret = 0;

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

	const bool attach_only = true; // db attach only, game create
	auto env = smd::Env<UniqsModel::DataCenter>::Create(SHMID_GAME_DB_USER, 25, attach_only);
	if (env == nullptr) {
		SMD_LOG_ERROR("Create env failed");
		return 0;
	}

	std::srand((unsigned int)std::time(nullptr));

	ret = main_db(env->GetEntry());

#ifdef _WIN32
	int n = 0;
	std::cin >> n;
#endif
	return ret;
}
