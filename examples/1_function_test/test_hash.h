#pragma once
#include <unordered_set>
#include <algorithm>
#include <smd.h>

class TestHash {
public:
	TestHash() {
		TestHashPod();
		TestHashString();
	}

private:
	void TestHashString() {
		auto mem_usage = smd::g_alloc->GetUsed();
		auto obj = smd::g_alloc->New<smd::shm_hash<smd::shm_string>>().Ptr();
		std::unordered_set<std::string> ref;

		std::vector<int> vRoleIds;
		const int COUNT = 1000;
		for (auto i = 0; i < COUNT; i++) {
			vRoleIds.push_back(i);
		}
		std::default_random_engine generator{ std::random_device{}() };
		std::shuffle(vRoleIds.begin(), vRoleIds.end(), generator);

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			auto key = GetKey(vRoleIds[i]);

			obj->insert(smd::shm_string(key));
			ref.insert(key);

			// 插入相同的数据，这两个map应该完全相同
			assert(IsEqual(*obj, ref));
		}

		assert(obj->size() == COUNT);
		assert(obj->find(GetKey(3)) != obj->end());
		assert(obj->find(GetKey(8)) != obj->end());
		assert(obj->find(GetKey(COUNT)) == obj->end());

		for (auto it = obj->begin(); it != obj->end(); ++it) {
			//const auto& k = *it;
			//log.DoLog(smd::Log::LogLevel::kInfo, "%s", k.data());
		}

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			auto key = GetKey(vRoleIds[i]);

			auto itm = obj->find(key);
			assert(itm != obj->end());
			obj->erase(itm);

			auto it_ref = ref.find(key);
			assert(it_ref != ref.end());
			ref.erase(it_ref);

			// 删除相同的数据，这两个map应该完全相同
			assert(IsEqual(*obj, ref));
		}

		assert(obj->find(GetKey(3)) == obj->end());
		assert(obj->find(GetKey(8)) == obj->end());
		assert(obj->find(GetKey(COUNT)) == obj->end());

		assert(obj->size() == 0);
		smd::g_alloc->Delete(obj);
		assert(obj == nullptr);
		ref.clear();

		assert(mem_usage == smd::g_alloc->GetUsed());
		SMD_LOG_INFO("TestHashString complete");
	}

	void TestHashPod() {
		auto mem_usage = smd::g_alloc->GetUsed();
		auto obj = smd::g_alloc->New<smd::shm_hash<uint64_t>>();
		std::unordered_set<uint64_t> ref;

		std::vector<uint64_t> vRoleIds;
		const size_t COUNT = 1000;
		for (size_t i = 0; i < COUNT; i++) {
			vRoleIds.push_back(i);
		}

		std::default_random_engine generator{ std::random_device{}() };
		std::shuffle(vRoleIds.begin(), vRoleIds.end(), generator);

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			const auto& role_id = vRoleIds[i];

			obj->insert(role_id);
			ref.insert(role_id);

			// 插入相同的数据，这两个map应该完全相同
			assert(IsEqual(*obj, ref));
		}

		assert(obj->size() == COUNT);
		assert(obj->find(3) != obj->end());
		assert(obj->find(8) != obj->end());
		assert(obj->find(COUNT) == obj->end());

		for (auto it = obj->begin(); it != obj->end(); ++it) {
			//const auto& k = *it;
			//log.DoLog(smd::Log::LogLevel::kInfo, "%llu", k);
		}

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			const auto& key = vRoleIds[i];

			auto itm = obj->find(key);
			assert(itm != obj->end());
			obj->erase(itm);

			auto it_ref = ref.find(key);
			assert(it_ref != ref.end());
			ref.erase(it_ref);

			// 删除相同的数据，这两个map应该完全相同
			assert(IsEqual(*obj, ref));
		}

		assert(obj->find(0) == obj->end());
		assert(obj->find(8) == obj->end());
		assert(obj->find(COUNT) == obj->end());

		assert(obj->size() == 0);

		smd::g_alloc->Delete(obj);
		assert(obj == smd::shm_nullptr);
		assert(mem_usage == smd::g_alloc->GetUsed());

		SMD_LOG_INFO("TestMapPod complete");
	}

private:

	static std::string GetKey(int key) {
		return Util::Text::Format("Key%05d", key);
	}

	// 这段代码可以优化下，这里主要用于测试
	bool IsEqual(smd::shm_hash<smd::shm_string>& l, const std::unordered_set<std::string>& r) {
		if (l.size() != r.size())
			return false;

		for (auto it = l.begin(); it != l.end(); ++it) {
			auto& key = *it;
			if (r.find(key.ToString()) == r.end()) {
				return false;
			}
		}

		for (auto it = r.begin(); it != r.end(); ++it) {
			const auto& key = *it;
			if (l.find(smd::shm_string(key)) == l.end()) {
				return false;
			}
		}

		return true;
	}

	// 这段代码可以优化下，这里主要用于测试
	bool IsEqual(smd::shm_hash<uint64_t>& l, const std::unordered_set<uint64_t>& r) {
		if (l.size() != r.size())
			return false;

		for (auto it = l.begin(); it != l.end(); ++it) {
			auto& key = *it;
			if (r.find(key) == r.end()) {
				return false;
			}
		}

		for (auto it = r.begin(); it != r.end(); ++it) {
			const auto& key = *it;
			if (l.find(key) == l.end()) {
				return false;
			}
		}

		return true;
	}
};
