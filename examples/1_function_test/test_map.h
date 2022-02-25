#pragma once
#include <map>
#include <smd.h>

class TestMap {
public:
	TestMap() {
		TestMapPod();
		TestMapString();
	}

private:
	void TestMapString() {
		auto mem_usage = smd::g_alloc->GetUsed();
		auto obj = smd::g_alloc->New<smd::shm_map<smd::shm_string, smd::shm_string>>().Ptr();
		std::map<std::string, std::string> ref;

		std::vector<int> vRoleIds;
		const size_t COUNT = 1000;
		for (size_t i = 0; i < COUNT; i++) {
			vRoleIds.push_back(i);
		}
		std::random_shuffle(vRoleIds.begin(), vRoleIds.end());

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			auto key = GetKey(vRoleIds[i]);
			auto value = GetValue(vRoleIds[i]);

			obj->insert(std::make_pair(key, value));
			ref.insert(std::make_pair(key, value));

			// 插入相同的数据，这两个map应该完全相同
			assert(IsEqual(*obj, ref));
		}

		assert(obj->size() == COUNT);
		assert(obj->find(GetKey(3)) != obj->end());
		assert(obj->find(GetKey(8)) != obj->end());
		assert(obj->find(GetKey(COUNT)) == obj->end());

		for (auto it = obj->begin(); it != obj->end(); ++it) {
			//const auto& k = it->first;
			//const auto& v = it->second;
			//log.DoLog(smd::Log::LogLevel::kInfo, "%s, %s", k.data(), v.data());
		}

		do {
			smd::shm_map<smd::shm_string, smd::shm_string> m(*obj);
			assert(m.size() == obj->size());
		} while (false);

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
		SMD_LOG_INFO("TestMapString complete");
	}

	void TestMapPod() {
		auto mem_usage = smd::g_alloc->GetUsed();
		auto obj = smd::g_alloc->New<smd::shm_map<uint64_t, uint64_t>>();
		std::map<uint64_t, uint64_t> ref;

		std::vector<uint64_t> vRoleIds;
		const size_t COUNT = 1000;
		for (size_t i = 0; i < COUNT; i++) {
			vRoleIds.push_back(i);
		}

		std::random_shuffle(vRoleIds.begin(), vRoleIds.end());

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			const auto& role_id = vRoleIds[i];
			auto value = role_id * 10;

			obj->insert(std::make_pair(role_id, value));
			ref.insert(std::make_pair(role_id, value));

			// 插入相同的数据，这两个map应该完全相同
			assert(IsEqual(*obj, ref));
		}

		assert(obj->size() == COUNT);
		assert(obj->find(3) != obj->end());
		assert(obj->find(8) != obj->end());
		assert(obj->find(COUNT) == obj->end());

		for (auto it = obj->begin(); it != obj->end(); ++it) {
			//const auto& k = it->first;
			//const auto& v = it->second;
			//log.DoLog(smd::Log::LogLevel::kInfo, "%llu, %llu", k, v);
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

	static std::string GetValue(int val) {
		return Util::Text::Format("Value%05d", val);
	}

	//测试专用
	static bool IsEqual(smd::shm_map<smd::shm_string, smd::shm_string>& l,
		const std::map<std::string, std::string>& r) {
		if (l.size() != r.size()) {
			assert(false);
		}

		auto it = l.begin();
		auto stl_it = r.begin();
		size_t count = 0;
		for (; it != l.end() && stl_it != r.end(); ++it, ++stl_it) {
			++count;
			const auto& key1 = it->first;
			const auto& key2 = stl_it->first;
			if (key1.ToString() != key2) {
				assert(false);
			}

			const auto& val1 = it->second;
			const auto& val2 = stl_it->second;
			if (val1.ToString() != val2) {
				assert(false);
			}
		}

		if (count != l.size()) {
			assert(false);
		}

		return true;
	}

	//测试专用
	static bool IsEqual(smd::shm_map<uint64_t, uint64_t>& l, const std::map<uint64_t, uint64_t>& r) {
		if (l.size() != r.size()) {
			assert(false);
		}

		auto it = l.begin();
		auto stl_it = r.begin();
		size_t count = 0;
		for (; it != l.end() && stl_it != r.end(); ++it, ++stl_it) {
			++count;
			const auto& key1 = it->first;
			const auto& key2 = stl_it->first;
			if (key1 != key2) {
				assert(false);
			}

			const auto& val1 = it->second;
			const auto& val2 = stl_it->second;
			if (val1 != val2) {
				assert(false);
			}
		}

		if (count != l.size()) {
			assert(false);
		}

		return true;
	}
};
