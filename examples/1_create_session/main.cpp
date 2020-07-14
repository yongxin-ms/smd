#include <stdio.h>
#include "smd.h"

int main() {
	const auto GUID("0x1001187fb");
	auto mgr = new smd::EnvMgr;
	auto env = mgr->CreateEnv(GUID, smd::EnvMgr::CREATE_IF_NOT_EXIST);

	std::string key("Alice");
	env->Set(key, "age 18");
	auto result = env->Get(key, nullptr);
	env->Del(key);

	return 0;
}
