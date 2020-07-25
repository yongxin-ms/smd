#include <stdio.h>
#include <iostream>
#include "smd.h"
#include "util.h"

void TestShmString(smd::Alloc& alloc) {
	auto mem_usage = alloc.GetUsed();
	auto s = alloc.New<smd::ShmString>(alloc, 16);
	assert(s->capacity() == 16);
	assert(s->size() == 0);
	assert(s->ToString() == "");

	s->assign("hello");
	assert(s->ToString() == "hello");

	*s = "1234567812345678helloaaaa";
	assert(s->ToString() == "1234567812345678helloaaaa");
	assert(s->capacity() == 32);
	s->clear();
	assert(s->ToString() == "");
	alloc.Delete(s);
	assert(s == nullptr);
	assert(mem_usage == alloc.GetUsed());
}

void TestShmList(smd::Alloc& alloc) {
	auto mem_usage = alloc.GetUsed();
	auto l = alloc.New<smd::ShmList<smd::ShmString>>(alloc);

	assert(l->size() == 0);
	l->push_back(smd::ShmString(alloc, "hello"));
	assert(l->size() == 1);
	assert(l->front().ToString() == "hello");
	assert(l->back().ToString() == "hello");

	l->push_back(smd::ShmString(alloc, "world"));
	assert(l->size() == 2);
	assert(l->front().ToString() == "hello");
	assert(l->back().ToString() == "world");

	l->push_back(smd::ShmString(alloc, "will"));
	assert(l->size() == 3);
	assert(l->front().ToString() == "hello");
	assert(l->back().ToString() == "will");

	alloc.Delete(l);
	assert(l == nullptr);
	assert(mem_usage == alloc.GetUsed());
}

int main() {
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

	const std::string GUID("0x1001187fb");
	//auto env = mgr->CreateEnv(GUID, 28, smd::create | smd::open);
	auto env = mgr->CreateEnv(GUID, 28, smd::create);
	assert(env != nullptr);

	TestShmString(env->GetMalloc());
	TestShmList(env->GetMalloc());

// 	std::string key("Alice");
// 	smd::Slice value;
// 	assert(!env->SGet(key, nullptr));
// 	assert(!env->SGet(key, &value));
// 	assert(!env->SDel(key));
// 
// 	env->SSet(key, "age 18");
// 	assert(env->SGet(key, nullptr));
// 	assert(env->SGet(key, &value));
// 	assert(value == "age 18");
// 	assert(env->SDel(key));
// 
// 	assert(!env->SGet(key, nullptr));
// 	assert(!env->SGet(key, &value));
// 	assert(!env->SDel(key));

	int n = 0;
	std::cin >> n;
	return 0;
}

