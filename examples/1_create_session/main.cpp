#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "smd.h"
#include "util.h"

void TestPointer(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();

	// 开辟共享内存
	smd::ShmPointer<int> shm_ptr = smd::g_alloc->New<int>();
	assert(shm_ptr != smd::shm_nullptr);

	// 指针在共享内存中的相对位置
	log.DoLog(smd::Log::LogLevel::kInfo, "Raw pointer:%lld", shm_ptr.Raw());

	// 访问
	*shm_ptr = 10;
	assert(*shm_ptr == 10);

	// 回收共享内存
	int* tmp = shm_ptr.Ptr();
	smd::g_alloc->Delete(tmp);
	assert(tmp == nullptr);

	// 没有内存泄露
	assert(mem_usage == smd::g_alloc->GetUsed());
	log.DoLog(smd::Log::LogLevel::kInfo, "TestPointer complete");
}

void TestArrayPointer(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();
	const size_t ARRAY_SIZE = 16;

	// 开辟共享内存
	smd::ShmPointer<int> shm_ptr = smd::g_alloc->Malloc<int>(ARRAY_SIZE);
	assert(shm_ptr != smd::shm_nullptr);

	// 指针在共享内存中的相对位置
	log.DoLog(smd::Log::LogLevel::kInfo, "Raw pointer:%lld", shm_ptr.Raw());

	// 随机访问
	for (int i = 0; i < ARRAY_SIZE; i++) {
		shm_ptr[i] = i * 10;
	}

	// 随机访问
	for (int i = 0; i < ARRAY_SIZE; i++) {
		log.DoLog(smd::Log::LogLevel::kInfo, "Array[%d] = %d", i, shm_ptr[i]);
	}

	//和普通指针一样可以++
	auto p = shm_ptr;
	for (int i = 0; i < ARRAY_SIZE; i++, p++) {
		log.DoLog(smd::Log::LogLevel::kInfo, "Array[%d] = %d", i, *p);
	}

	// 回收共享内存
	smd::g_alloc->Free(shm_ptr, ARRAY_SIZE);
	assert(shm_ptr == smd::shm_nullptr);

	// 没有内存泄露
	assert(mem_usage == smd::g_alloc->GetUsed());
	log.DoLog(smd::Log::LogLevel::kInfo, "TestPointer complete");
}

void TestPointerToObject(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();

	struct StA {
		int a1;
		int a2;
	};

	class StB {
	public:
		StB() { m_pointer_a = smd::g_alloc->New<StA>(); }
		~StB() { smd::g_alloc->Delete(m_pointer_a); }

		smd::ShmPointer<StA> m_pointer_a;
		int m_b1;
	};

	// 开辟共享内存
	smd::ShmPointer<StB> shm_ptr = smd::g_alloc->New<StB>();
	assert(shm_ptr != smd::shm_nullptr);

	// 指针在共享内存中的相对位置
	log.DoLog(smd::Log::LogLevel::kInfo, "Raw pointer:%lld", shm_ptr.Raw());

	// 访问
	shm_ptr->m_b1 = 10;
	shm_ptr->m_pointer_a->a1 = 5;
	shm_ptr->m_pointer_a->a2 = 127;

	assert(shm_ptr->m_b1 == 10);
	assert(shm_ptr->m_pointer_a->a1 == 5);
	assert(shm_ptr->m_pointer_a->a2 == 127);

	// 回收共享内存
	smd::g_alloc->Delete(shm_ptr);
	assert(shm_ptr == smd::shm_nullptr);

	// 没有内存泄露
	assert(mem_usage == smd::g_alloc->GetUsed());
	log.DoLog(smd::Log::LogLevel::kInfo, "TestPointer complete");
}

void TestShmString(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();
	auto s = smd::g_alloc->New<smd::ShmString>(16);
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
	auto t = smd::g_alloc->New<smd::ShmString>(32);

	t->assign("world");
	assert(t->ToString() == "world");
	assert(s->ToString() == "hello");
	smd::g_alloc->Delete(t);
	assert(t == smd::shm_nullptr);
	assert(s->ToString() == "hello");

	// 验证拷贝构造函数
	t = smd::g_alloc->New<smd::ShmString>(*s);
	assert(t->data() != s->data());
	assert(t->ToString() == "hello");
	smd::g_alloc->Delete(t);
	assert(t == smd::shm_nullptr);
	assert(s->ToString() == "hello");

	// 验证拷贝构造函数
	t = smd::g_alloc->New<smd::ShmString>("hello");
	assert(t->data() != s->data());
	assert(t->ToString() == "hello");
	smd::g_alloc->Delete(t);
	assert(t == smd::shm_nullptr);
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
	smd::g_alloc->Delete(s);
	assert(s == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());

	log.DoLog(smd::Log::LogLevel::kInfo, "TestShmString complete");
}

void TestShmList(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();

	auto l = smd::g_alloc->New<smd::ShmList<smd::ShmString>>();

	// 验证在尾部添加元素
	assert(l->size() == 0);
	l->push_back(smd::ShmString("hello"));
	assert(l->size() == 1);
	assert(l->front().ToString() == "hello");
	assert(l->back().ToString() == "hello");

	do {

		// 验证在尾部添加多个元素
		for (int i = 0; i < 100; i++) {
			l->push_back(smd::ShmString(Util::Text::Format("TestText%02d", i)));
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
			l->push_front(smd::ShmString(Util::Text::Format("TestText%02d", i)));
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
			l->push_back(smd::ShmString(Util::Text::Format("TestText%02d", i)));
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

	do {
		smd::ShmList<smd::ShmString> l1(*l);
		// 验证在尾部添加多个元素
		for (int i = 0; i < 100; i++) {
			l1.push_back(smd::ShmString(Util::Text::Format("TestText%02d", i)));
		}
		assert(l1.size() == 101);
		assert(l1.front().ToString() == "hello");
		assert(l1.back().ToString() == "TestText99");

		smd::ShmList<smd::ShmString> l2;
		l2 = *l;
		// 验证在尾部添加多个元素
		for (int i = 0; i < 100; i++) {
			l2.push_back(smd::ShmString(Util::Text::Format("TestText%02d", i)));
		}
		assert(l2.size() == 101);
		assert(l2.front().ToString() == "hello");
		assert(l2.back().ToString() == "TestText99");
	} while (false);

	l->push_back(smd::ShmString("world"));
	assert(l->size() == 2);
	assert(l->front().ToString() == "hello");
	assert(l->back().ToString() == "world");

	l->push_back(smd::ShmString("will"));
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

	smd::g_alloc->Delete(l);
	assert(l == smd::shm_nullptr);

	assert(mem_usage == smd::g_alloc->GetUsed());
	log.DoLog(smd::Log::LogLevel::kInfo, "TestShmList complete");
}

void TestShmListPod(smd::Log& log) {
	struct StMyData {
		uint64_t role_id_;
		int hp_;

		StMyData(uint64_t role_id = 0, int hp = 0)
			: role_id_(role_id)
			, hp_(hp) {}
	};

	auto mem_usage = smd::g_alloc->GetUsed();
	auto l = smd::g_alloc->New<smd::ShmList<StMyData>>();

	// 验证在尾部添加元素
	assert(l->size() == 0);
	l->push_back(StMyData(7131, 14));
	assert(l->size() == 1);
	// auto& element = l->front();

	smd::g_alloc->Delete(l);
	assert(l == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());
	log.DoLog(smd::Log::LogLevel::kInfo, "TestShmListPod complete");
}

void TestShmVector(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();
	auto v = smd::g_alloc->New<smd::ShmVector<smd::ShmString>>();

	assert(v->size() == 0);
	v->push_back(smd::ShmString("hello"));
	assert(v->size() == 1);
	assert(v->front().ToString() == "hello");
	assert(v->back().ToString() == "hello");

	do {
		smd::ShmVector<smd::ShmString> v1(*v);
		for (int i = 0; i < 100; ++i) {
			v1.push_back(smd::ShmString(Util::Text::Format("TestText%02d", i)));
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
			v1.push_back(smd::ShmString(Util::Text::Format("TestText%02d", i)));
		}
		assert(v1.size() == 101);
		assert(v1[0].ToString() == "hello");
		assert(v1[100].ToString() == "TestText99");

		v1.clear();
		assert(v1.size() == 0);

		v1 = *v;
		assert(v1.size() == 1);
		assert(v1[0].ToString() == "hello");

		v1.resize(300, smd::ShmString("123"));
		assert(v1.size() == 300);
		assert(v1[299].ToString() == "123");

	} while (false);

	v->push_back(smd::ShmString("world"));
	assert(v->size() == 2);
	assert(v->front().ToString() == "hello");
	assert(v->back().ToString() == "world");

	v->push_back(smd::ShmString("will"));
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

	smd::g_alloc->Delete(v);
	assert(v == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());

	log.DoLog(smd::Log::LogLevel::kInfo, "TestShmVector complete");
}

void TestShmVectorResize(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();
	auto v = smd::g_alloc->New<smd::ShmVector<smd::ShmString>>(64);
	v->resize(v->capacity(), smd::ShmString(""));

	smd::g_alloc->Delete(v);
	assert(v == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());

	log.DoLog(smd::Log::LogLevel::kInfo, "TestShmVectorResize complete");
}

void TestShmVectorPod(smd::Log& log) {
	struct StMyData {
		uint64_t role_id_;
		int hp_;

		StMyData(uint64_t role_id = 0, int hp = 0)
			: role_id_(role_id)
			, hp_(hp) {}
	};

	auto mem_usage = smd::g_alloc->GetUsed();
	auto v = smd::g_alloc->New<smd::ShmVector<StMyData>>();

	// 验证在尾部添加元素
	assert(v->size() == 0);
	v->push_back(StMyData(7131, 14));
	assert(v->size() == 1);
	// auto& element = v->front();

	smd::g_alloc->Delete(v);
	assert(v == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());
	log.DoLog(smd::Log::LogLevel::kInfo, "TestShmVectorPod complete");
}

void TestHash(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();
	auto h = smd::g_alloc->New<smd::ShmHash<smd::ShmString>>();

	assert(h->size() == 0);
	h->insert(smd::ShmString("hello"));
	assert(h->size() == 1);
	assert((*h->begin()).ToString() == "hello");
	assert(h->count(smd::ShmString("hello")));
	h->clear();

	h->insert(smd::ShmString("hello"));
	h->insert(smd::ShmString("world"));
	assert(h->size() == 2);
	assert((*h->begin()).ToString() == "hello");
	assert(h->count(smd::ShmString("world")));

	h->insert(smd::ShmString("will"));
	assert(h->size() == 3);
	assert((*h->begin()).ToString() == "hello");
	assert(h->count(smd::ShmString("will")));

	for (auto it = h->begin(); it != h->end(); ++it) {
		log.DoLog(smd::Log::LogLevel::kInfo, "%s", it->ToString().data());
	}

	for (auto it = h->begin(); it != h->end();) {
		it = h->erase(it);
	}

	assert(h->find(smd::ShmString("hello")) == h->end());
	assert(h->find(smd::ShmString("world")) == h->end());
	assert(h->find(smd::ShmString("will")) == h->end());

	assert(h->size() == 0);
	smd::g_alloc->Delete(h);
	assert(h == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());

	log.DoLog(smd::Log::LogLevel::kInfo, "TestHash complete");
}

void TestHashPod(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();
	auto h = smd::g_alloc->New<smd::ShmHash<uint64_t>>();
	auto h_ref = smd::g_alloc->New<smd::ShmHash<uint64_t>>();

	std::vector<uint64_t> vRoleIds;
	for (size_t i = 0; i < 1000; i++) {
		vRoleIds.push_back(i);
		h_ref->insert(i);
	}

	random_shuffle(vRoleIds.begin(), vRoleIds.end());

	for (size_t i = 0; i < vRoleIds.size(); i++) {
		const auto& role_id = vRoleIds[i];
		h->insert(role_id);
	}

	assert(h->size() == 1000);
	assert(*h == *h_ref);
	assert(h->find(1) != h->end());
	assert(h->find(8) != h->end());
	assert(h->find(1000) == h->end());

	for (auto it = h->begin(); it != h->end(); ++it) {
		log.DoLog(smd::Log::LogLevel::kInfo, "%llu", *it);
	}

	for (auto it = h->begin(); it != h->end();) {
		it = h->erase(it);
	}

	assert(h->size() == 0);
	assert(h->find(1) == h->end());
	assert(h->find(8) == h->end());
	assert(h->find(1000) == h->end());

	assert(h->size() == 0);
	smd::g_alloc->Delete(h);
	assert(h == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());

	log.DoLog(smd::Log::LogLevel::kInfo, "TestHashPod complete");
}

void TestMapString(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();
	auto m = smd::g_alloc->New<smd::ShmMap<smd::ShmString, smd::ShmString>>();

	const size_t COUNT = 100;
	for (size_t i = 0; i < COUNT; ++i) {
		Util::Text::Format("TestText%02d", i);
		auto key = smd::ShmString(Util::Text::Format("Key%02d", i));
		auto value = smd::ShmString(Util::Text::Format("Value%02d", i));
		m->insert(smd::make_pair(key, value));
	}

	assert(m->size() == COUNT);
	assert(m->find(smd::ShmString(Util::Text::Format("Key%02d", 3))) != m->end());
	assert(m->find(smd::ShmString(Util::Text::Format("Key%02d", 8))) != m->end());
	assert(m->find(smd::ShmString(Util::Text::Format("Key%02d", COUNT))) == m->end());

	for (auto it = m->begin(); it != m->end(); ++it) {
		const auto& k = it->first;
		const auto& v = it->second;
		log.DoLog(smd::Log::LogLevel::kInfo, "%s, %s", k.data(), v.data());
	}

	m->clear();

	assert(m->find(smd::ShmString(Util::Text::Format("Key%02d", 3))) == m->end());
	assert(m->find(smd::ShmString(Util::Text::Format("Key%02d", 8))) == m->end());
	assert(m->find(smd::ShmString(Util::Text::Format("Key%02d", COUNT))) == m->end());

	assert(m->size() == 0);
	smd::g_alloc->Delete(m);
	assert(m == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());

	log.DoLog(smd::Log::LogLevel::kInfo, "TestMapString complete");
}

void TestMapPod(smd::Log& log) {
	auto mem_usage = smd::g_alloc->GetUsed();
	auto m = smd::g_alloc->New<smd::ShmMap<uint64_t, uint64_t>>();

	std::vector<uint64_t> vRoleIds;
	const size_t COUNT = 100;
	for (size_t i = 0; i < COUNT; i++) {
		vRoleIds.push_back(i);
	}

	std::random_shuffle(vRoleIds.begin(), vRoleIds.end());

	for (size_t i = 0; i < vRoleIds.size(); i++) {
		const auto& role_id = vRoleIds[i];
		m->insert(smd::make_pair(role_id, role_id * 10));
	}

	assert(m->size() == COUNT);
	assert(m->find(3) != m->end());
	assert(m->find(8) != m->end());
	assert(m->find(COUNT) == m->end());

	for (auto it = m->begin(); it != m->end(); ++it) {
		const auto& k = it->first;
		const auto& v = it->second;
		log.DoLog(smd::Log::LogLevel::kInfo, "%llu, %llu", k, v);
	}

	m->clear();

	assert(m->find(0) == m->end());
	assert(m->find(8) == m->end());
	assert(m->find(COUNT) == m->end());

	assert(m->size() == 0);
	m->clear();

	smd::g_alloc->Delete(m);
	assert(m == smd::shm_nullptr);
	assert(mem_usage == smd::g_alloc->GetUsed());

	log.DoLog(smd::Log::LogLevel::kInfo, "TestMapPod complete");
}

int main() {
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
	// auto env = mgr->CreateEnv(GUID, 20, smd::kOpenExist);
	auto env = mgr->CreateEnv(GUID, 20, smd::kCreateAlways);
	assert(env != nullptr);
	auto& log = env->GetLog();

	TestPointer(log);
	TestArrayPointer(log);
	TestPointerToObject(log);
	TestShmString(log);
	TestShmList(log);
	TestShmListPod(log);
	TestShmVector(log);
	TestShmVectorResize(log);
	TestShmVectorPod(log);
	TestHash(log);
	TestHashPod(log);
	TestMapString(log);
	TestMapPod(log);

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

	// auto& all_strings = env->GetAllStrings();
	auto& all_strings = *smd::g_alloc->New<smd::ShmMap<smd::ShmString, smd::ShmString>>();

	// 	all_strings.insert(make_pair(smd::ShmString("will1"), smd::ShmString("1")));
	// 	all_strings.insert(make_pair(smd::ShmString("will2"), smd::ShmString("2")));
	// 	all_strings.insert(make_pair(smd::ShmString("will3"), smd::ShmString("3")));
	// 	all_strings.insert(make_pair(smd::ShmString("will4"), smd::ShmString("4")));
	// 	all_strings.insert(make_pair(smd::ShmString("will5"), smd::ShmString("5")));
	// 	all_strings.insert(make_pair(smd::ShmString("will6"), smd::ShmString("6")));

	for (int i = 0; i < 20; i++) {
		std::string key1 = Util::Text::Format("Hello%03d%03d", count, i);
		std::string value1 = Util::Text::Format("World%03d", i);
		all_strings.insert(smd::make_pair(smd::ShmString(key1), smd::ShmString(value1)));
	}

	for (auto it = all_strings.begin(); it != all_strings.end(); ++it) {
		const auto& key = it->first;
		const auto& value = it->second;

		log.DoLog(smd::Log::LogLevel::kInfo, "Key:%s Value:%s", key.ToString().data(),
			value.ToString().data());
	}

	int n = 0;
	std::cin >> n;
	return 0;
}
