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

	const char* ori_ptr = s->data();
	s->assign("hello");
	assert(s->size() == strlen("hello"));
	assert(s->ToString() == "hello");

	//发生了扩容
	s->assign("hellohellohellohellohellohellohellohellohellohello");
	assert(s->data() != ori_ptr);
	s->assign("hello");

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
	*s = "1234567812345678helloaaaafdsfdsddddddddddddddddddddddddddddddddddddddddddd";
	alloc.Delete(s);
	assert(s == nullptr);
	assert(mem_usage == alloc.GetUsed());

	env->GetLog().DoLog(smd::Log::LogLevel::kInfo, "String test complete");
}

void TestShmList(smd::Env* env) {
	auto& alloc		= env->GetMalloc();
	auto mem_usage = alloc.GetUsed();
	auto l = alloc.New<smd::ShmList<smd::ShmString>>(alloc);

	// 验证在尾部添加元素
	assert(l->size() == 0);
	l->push_back(smd::ShmString(alloc, "hello"));
	assert(l->size() == 1);
	assert(l->front().ToString() == "hello");
	assert(l->back().ToString() == "hello");

	do  {

		// 验证在尾部添加多个元素
		for (int i = 0; i < 100; i++) {
			l->push_back(smd::ShmString(alloc, Util::Text::Format("TestText%02d", i)));
		}
		assert(l->size() == 101);
		assert(l->front().ToString() == "hello");
		assert(l->back().ToString() == "TestText99");

		// 验证在尾部删除多个元素
		for (int i = 0; i < 100; i++) {
			l->pop_back();
		}
		assert(l->size() == 1);
		assert(l->front().ToString() == "hello");
		assert(l->back().ToString() == "hello");

		// 验证在头部添加多个元素
		for (int i = 0; i < 100; i++) {
			l->push_front(smd::ShmString(alloc, Util::Text::Format("TestText%02d", i)));
		}
		assert(l->size() == 101);
		assert(l->front().ToString() == "TestText99");
		assert(l->back().ToString() == "hello");

		// 验证在头部删除多个元素
		for (int i = 0; i < 100; i++) {
			l->pop_front();
		}
		assert(l->size() == 1);
		assert(l->front().ToString() == "hello");
		assert(l->back().ToString() == "hello");

		// 验证在尾部添加多个元素
		for (int i = 0; i < 100; i++) {
			l->push_back(smd::ShmString(alloc, Util::Text::Format("TestText%02d", i)));
		}
		assert(l->size() == 101);
		assert(l->front().ToString() == "hello");
		assert(l->back().ToString() == "TestText99");

		// 验证删除元素
		for (auto it = l->begin(); it != l->end();) {
			if (it->ToString().find("TestText") != std::string::npos) {
				it = l->erase(it);
			} else {
				++it;
			}
		}
		assert(l->size() == 1);
		assert(l->front().ToString() == "hello");
		assert(l->back().ToString() == "hello");
	} while (false);

	do  {
		smd::ShmList<smd::ShmString> l1(*l);
		// 验证在尾部添加多个元素
		for (int i = 0; i < 100; i++) {
			l1.push_back(smd::ShmString(alloc, Util::Text::Format("TestText%02d", i)));
		}
		assert(l1.size() == 101);
		assert(l1.front().ToString() == "hello");
		assert(l1.back().ToString() == "TestText99");

		smd::ShmList<smd::ShmString> l2(env->GetMalloc());
		l2 = *l;
		// 验证在尾部添加多个元素
		for (int i = 0; i < 100; i++) {
			l2.push_back(smd::ShmString(alloc, Util::Text::Format("TestText%02d", i)));
		}
		assert(l2.size() == 101);
		assert(l2.front().ToString() == "hello");
		assert(l2.back().ToString() == "TestText99");
	} while (false);

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

void TestShmVector(smd::Env* env) {
	auto& alloc		= env->GetMalloc();
	auto mem_usage = alloc.GetUsed();
	auto v		   = alloc.New<smd::ShmVector<smd::ShmString>>(alloc);

	assert(v->size() == 0);
	v->push_back(smd::ShmString(alloc, "hello"));
	assert(v->size() == 1);
	assert(v->front().ToString() == "hello");
	assert(v->back().ToString() == "hello");

	do {
		smd::ShmVector<smd::ShmString> v1(*v);
		for (int i = 0; i < 100; ++i) {
			v1.push_back(smd::ShmString(alloc, Util::Text::Format("TestText%02d", i)));
		}
		assert(v1.size() == 101);
		assert(v1[0].ToString() == "hello");
		assert(v1[100].ToString() == "TestText99");

		for (int i = 0; i < 100; ++i) {
			v1.pop_back();
		}
		assert(v1.size() == 1);
		assert(v1[0].ToString() == "hello");

		for (int i = 0; i < 100; ++i) {
			v1.push_back(smd::ShmString(alloc, Util::Text::Format("TestText%02d", i)));
		}
		assert(v1.size() == 101);
		assert(v1[0].ToString() == "hello");
		assert(v1[100].ToString() == "TestText99");

		v1.clear();
		assert(v1.size() == 0);

		v1 = *v;
		assert(v1.size() == 1);
		assert(v1[0].ToString() == "hello");

		v1.resize(300, smd::ShmString(env->GetMalloc(), "123"));
		assert(v1.size() == 300);
		assert(v1[299].ToString() == "123");

	} while (false);


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

	assert(h->size() == 0);
	h->insert(smd::ShmString(alloc, "hello"));
	assert(h->size() == 1);
	assert((*h->begin()).ToString() == "hello");
	assert(h->count(smd::ShmString(alloc, "hello")));
	h->clear();

	h->insert(smd::ShmString(alloc, "hello"));
	h->insert(smd::ShmString(alloc, "world"));
	assert(h->size() == 2);
	assert((*h->begin()).ToString() == "hello");
	assert(h->count(smd::ShmString(alloc, "world")));

	h->insert(smd::ShmString(alloc, "will"));
	assert(h->size() == 3);
	assert((*h->begin()).ToString() == "hello");
	assert(h->count(smd::ShmString(alloc, "will")));

	for (auto it = h->begin(); it != h->end(); ++it) {
		env->GetLog().DoLog(smd::Log::LogLevel::kDebug, "%s", it->ToString().data());
	}

	for (auto it = h->begin(); it != h->end();) {
		it = h->erase(it);
	}

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
	TestShmVector(env);
	TestShmVectorResize(env);
	TestHash(env);

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

