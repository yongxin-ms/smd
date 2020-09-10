#pragma once
#include "smd.h"

class TestVector {
public:
	TestVector(smd::Log& log) {
		TestShmVector(log);
		TestShmVectorResize(log);
		TestShmVectorPod(log);
	}

private:
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

private:
	//测试专用
	bool IsEqual(smd::ShmVector<smd::ShmString>& l, const std::vector<std::string>& r) {
		if (l.size() != r.size()) {
			assert(false);
		}

		for (size_t i = 0; i < l.size(); i++) {
			const auto& key1 = l.operator[](i);
			const auto& key2 = r[i];
			if (key1 != key2) {
				assert(false);
			}
		}

		return true;
	}
};
