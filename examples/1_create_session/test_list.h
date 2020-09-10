#pragma once
#include <list>
#include "smd.h"

class TestList {
public:
	TestList(smd::Log& log) {
		TestShmList(log);
		TestListEqual(log);
		TestShmListPod(log);
	}

private:
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

	void TestListEqual(smd::Log& log) {
		auto mem_usage = smd::g_alloc->GetUsed();
		auto l = smd::g_alloc->New<smd::ShmList<smd::ShmString>>().Ptr();
		std::list<std::string> list_ref;

		std::vector<int> vRoleIds;
		const size_t COUNT = 1000;
		for (size_t i = 0; i < COUNT; i++) {
			vRoleIds.push_back(i);
		}

		std::random_shuffle(vRoleIds.begin(), vRoleIds.end());

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			auto key = Util::Text::Format("Key%05d", vRoleIds[i]);

			l->push_back(key);
			list_ref.push_back(key);
			assert(IsEqual(*l, list_ref));
		}

		smd::g_alloc->Delete(l);
		assert(l == nullptr);
		list_ref.clear();

		assert(mem_usage == smd::g_alloc->GetUsed());
		log.DoLog(smd::Log::LogLevel::kInfo, "TestListEqual complete");
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

private:
	//测试专用
	bool IsEqual(smd::ShmList<smd::ShmString>& l, const std::list<std::string>& r) {
		if (l.size() != r.size()) {
			assert(false);
		}

		auto it = l.begin();
		auto stl_it = r.begin();
		size_t count = 0;
		for (; it != l.end() && stl_it != r.end(); ++it, ++stl_it) {
			++count;
			const auto& key1 = *it;
			const auto& key2 = *stl_it;
			if (key1 != key2) {
				assert(false);
			}
		}

		if (count != l.size()) {
			assert(false);
		}

		return true;
	}
};
