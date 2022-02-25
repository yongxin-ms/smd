#pragma once
#include <smd.h>

class TestString {
public:
	TestString() {
		TestShmString();
	}

private:
	void TestShmString() {
		auto mem_usage = smd::g_alloc->GetUsed();
		auto s = smd::g_alloc->New<smd::shm_string>();
		//assert(s->capacity() > 16);
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
		auto t = smd::g_alloc->New<smd::shm_string>(32);

		t->assign("world");
		assert(t->ToString() == "world");
		assert(s->ToString() == "hello");
		smd::g_alloc->Delete(t);
		assert(t == smd::shm_nullptr);
		assert(s->ToString() == "hello");

		// 验证拷贝构造函数
		t = smd::g_alloc->New<smd::shm_string>(*s);
		assert(t->data() != s->data());
		assert(t->ToString() == "hello");
		smd::g_alloc->Delete(t);
		assert(t == smd::shm_nullptr);
		assert(s->ToString() == "hello");

		// 验证拷贝构造函数
		t = smd::g_alloc->New<smd::shm_string>("hello");
		assert(t->data() != s->data());
		assert(t->ToString() == "hello");
		smd::g_alloc->Delete(t);
		assert(t == smd::shm_nullptr);
		assert(s->ToString() == "hello");

		do {
			// 验证拷贝构造函数
			smd::shm_string t1(*s);
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

		SMD_LOG_INFO("TestShmString complete");
	}
};
