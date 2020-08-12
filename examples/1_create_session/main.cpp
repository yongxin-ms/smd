#include <stdio.h>
#include <iostream>
#include "smd.h"
#include "util.h"

void TestShmString(smd::Env* env) {
	auto& alloc		= env->GetMalloc();
	auto mem_usage = alloc.GetUsed();
	auto s = alloc.New<smd::ShmString>(alloc, 16);
	assert(s->capacity() > 16);
	assert(s->size() == 0);
	assert(s->ToString() == "");

	s->assign("hello");
	assert(s->size() == strlen("hello"));
	assert(s->ToString() == "hello");

	// 验证两个对象的创建互不影响
	auto t = alloc.New<smd::ShmString>(alloc, 32);
	t->assign("world");
	assert(t->ToString() == "world");
	assert(s->ToString() == "hello");
	alloc.Delete(t);
	assert(t == nullptr);
	assert(s->ToString() == "hello");

	// 验证拷贝构造函数
	t = alloc.New<smd::ShmString>(*s);
	assert(t->data() != s->data());
	assert(t->ToString() == "hello");
	alloc.Delete(t);
	assert(t == nullptr);
	assert(s->ToString() == "hello");

	// 验证拷贝构造函数
	t = alloc.New<smd::ShmString>(alloc, std::string("hello"));
	assert(t->data() != s->data());
	assert(t->ToString() == "hello");
	alloc.Delete(t);
	assert(t == nullptr);
	assert(s->ToString() == "hello");

	do {
		// 验证拷贝构造函数
		smd::ShmString t1(*s);
		assert(t1.data() != s->data());
		assert(t1.ToString() == s->ToString());

		// 验证赋值函数
		t1 = "312423232";
		assert(t1.ToString() == std::string("312423232"));
	} while (false);

	// 验证清除函数
	*s = "1234567812345678helloaaaa";
	assert(s->ToString() == "1234567812345678helloaaaa");
 	s->clear();
 	assert(s->ToString() == "");
	*s = "1234567812345678helloaaaafdsfds";
	alloc.Delete(s);
	assert(s == nullptr);
	assert(mem_usage == alloc.GetUsed());

	env->GetLog().DoLog(smd::Log::LogLevel::kInfo, "String test complete");
}

void TestShmList(smd::Env* env) {
	auto& alloc		= env->GetMalloc();
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

	for (auto it = l->begin(); it != l->end();) {
		if (it->ToString() == "world") {
			it = l->erase(it);
		} else {
			++it;
		}
	}

	alloc.Delete(l);
	assert(l == nullptr);
	assert(mem_usage == alloc.GetUsed());
	env->GetLog().DoLog(smd::Log::LogLevel::kInfo, "List test complete");
}

void TestShmListPushFront(smd::Env* env) {
	auto& alloc		= env->GetMalloc();
	auto mem_usage = alloc.GetUsed();
	auto l		   = alloc.New<smd::ShmList<smd::ShmString>>(alloc);

	assert(l->size() == 0);
	l->push_front(smd::ShmString(alloc, "hello"));
	assert(l->size() == 1);
	assert(l->front().ToString() == "hello");
	assert(l->back().ToString() == "hello");

	l->push_front(smd::ShmString(alloc, "world"));
	assert(l->size() == 2);
	assert(l->front().ToString() == "world");
	assert(l->back().ToString() == "hello");

	l->push_front(smd::ShmString(alloc, "will"));
	assert(l->size() == 3);
	assert(l->front().ToString() == "will");
	assert(l->back().ToString() == "hello");

	for (auto it = l->begin(); it != l->end();) {
		if (it->ToString() == "world") {
			it = l->erase(it);
		} else {
			++it;
		}
	}

	alloc.Delete(l);
	assert(l == nullptr);
	assert(mem_usage == alloc.GetUsed());

	env->GetLog().DoLog(smd::Log::LogLevel::kInfo, "ListPushFront test complete");
}


void TestShmVector(smd::Env* env) {
	auto& alloc		= env->GetMalloc();
	auto mem_usage = alloc.GetUsed();
	auto v		   = alloc.New<smd::ShmVector<smd::ShmString>>(alloc);

	assert(v->size() == 0);
	v->push_back(smd::ShmString(alloc, "hello"));
	assert(v->size() == 1);
	assert(v->front().ToString() == "hello");
	assert(v->back().ToString() == "hello");

	v->push_back(smd::ShmString(alloc, "world"));
	assert(v->size() == 2);
	assert(v->front().ToString() == "hello");
	assert(v->back().ToString() == "world");

	v->push_back(smd::ShmString(alloc, "will"));
	assert(v->size() == 3);
	assert(v->front().ToString() == "hello");
	assert(v->back().ToString() == "will");

	v->pop_back();
	assert(v->size() == 2);
	assert(v->front().ToString() == "hello");
	assert(v->back().ToString() == "world");

	v->pop_back();
	assert(v->size() == 1);
	assert(v->front().ToString() == "hello");
	assert(v->back().ToString() == "hello");

	v->pop_back();
	assert(v->size() == 0);

	alloc.Delete(v);
	assert(v == nullptr);
	assert(mem_usage == alloc.GetUsed());

	env->GetLog().DoLog(smd::Log::LogLevel::kInfo, "Vector test complete");
}

void TestShmVectorResize(smd::Env* env) {
	auto& alloc		= env->GetMalloc();
	auto  mem_usage = alloc.GetUsed();
	auto  v			= alloc.New<smd::ShmVector<smd::ShmString>>(alloc, 64);
	v->resize(v->capacity(), smd::ShmString(alloc));

	alloc.Delete(v);
	assert(v == nullptr);
	assert(mem_usage == alloc.GetUsed());

	env->GetLog().DoLog(smd::Log::LogLevel::kInfo, "VectorResize test complete");
}

void TestHash(smd::Env* env) {
	auto& alloc	  = env->GetMalloc();
	auto mem_usage = alloc.GetUsed();
	auto h		   = alloc.New<smd::ShmHash<smd::ShmString>>(alloc);

// 	assert(h->size() == 0);
// 	h->insert(smd::ShmString(alloc, "hello"));
// 	assert(h->size() == 1);
// 	assert((*h->begin()).ToString() == "hello");
// 	assert(h->count(smd::ShmString(alloc, "hello")));
// 	h->clear();

// 	h->insert(smd::ShmString(alloc, "world"));
// 	assert(h->size() == 2);
// 	assert((*h->begin()).ToString() == "hello");
// 	assert(h->count(smd::ShmString(alloc, "world")));
// 
// 	h->insert(smd::ShmString(alloc, "will"));
// 	assert(h->size() == 3);
// 	assert((*h->begin()).ToString() == "hello");
// 	assert(h->count(smd::ShmString(alloc, "will")));
// 
// 	for (auto it = h->begin(); it != h->end(); ++it) {
// 		env->GetLog().DoLog(smd::Log::LogLevel::kDebug, "%s", it->ToString().data());
// 	}
// 
// 	for (auto it = h->begin(); it != h->end();) {
// 		it = h->erase(it);
// 	}

	assert(h->size() == 0);
	alloc.Delete(h);
	assert(h == nullptr);
	assert(mem_usage == alloc.GetUsed());

	env->GetLog().DoLog(smd::Log::LogLevel::kInfo, "Hash test complete");
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
	auto env = mgr->CreateEnv(GUID, 20, smd::create);
	assert(env != nullptr);

	TestShmString(env);
	TestShmList(env);
// 	TestShmListPushFront(env);
// 	TestShmVector(env);
// 	TestShmVectorResize(env);
// 	TestHash(env);

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

