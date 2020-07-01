#include <stdio.h>
#include "smd.h"

int main() {
	const auto GUID("0x1001187fb");
	auto session = smd::CreateSession(GUID);
	session->ResetAll();

	std::string key("Alice");
	session->Set(key, "age 18");
	auto result = session->Get(key);
	session->Del(key);

	return 0;
}
