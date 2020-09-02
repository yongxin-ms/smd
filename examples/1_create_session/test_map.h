#pragma once
#include "smd.h"

class TestMap {
public:
	TestMap(smd::Log& log) {
		TestMapString(log);
		TestMapPod(log);
	}

private:
	void TestMapString(smd::Log& log) {
		auto mem_usage = smd::g_alloc->GetUsed();
		auto m = smd::g_alloc->New<smd::ShmMap<smd::ShmString, smd::ShmString>>().Ptr();
		std::map<smd::ShmString, smd::ShmString> map_ref;

		std::vector<int> vRoleIds;
		const size_t COUNT = 1000;
		for (size_t i = 0; i < COUNT; i++) {
			vRoleIds.push_back(i);
		}

		std::random_shuffle(vRoleIds.begin(), vRoleIds.end());

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			auto key = smd::ShmString(Util::Text::Format("Key%05d", vRoleIds[i]));
			auto value = smd::ShmString(Util::Text::Format("Value%05d", vRoleIds[i]));

			m->insert(smd::make_pair(key, value));
			map_ref.insert(std::make_pair(key, value));

			assert(m->IsEqual(map_ref));
		}

		assert(m->size() == COUNT);
		assert(m->find(smd::ShmString(Util::Text::Format("Key%05d", 3))) != m->end());
		assert(m->find(smd::ShmString(Util::Text::Format("Key%05d", 8))) != m->end());
		assert(m->find(smd::ShmString(Util::Text::Format("Key%05d", COUNT))) == m->end());

		for (auto it = m->begin(); it != m->end(); ++it) {
			const auto& k = it->first;
			const auto& v = it->second;
			log.DoLog(smd::Log::LogLevel::kInfo, "%s, %s", k.data(), v.data());
		}

		m->clear();

		assert(m->find(smd::ShmString(Util::Text::Format("Key%05d", 3))) == m->end());
		assert(m->find(smd::ShmString(Util::Text::Format("Key%05d", 8))) == m->end());
		assert(m->find(smd::ShmString(Util::Text::Format("Key%05d", COUNT))) == m->end());

		assert(m->size() == 0);
		smd::g_alloc->Delete(m);
		assert(m == nullptr);
		map_ref.clear();

		assert(mem_usage == smd::g_alloc->GetUsed());
		log.DoLog(smd::Log::LogLevel::kInfo, "TestMapString complete");
	}

	void TestMapPod(smd::Log& log) {
		auto mem_usage = smd::g_alloc->GetUsed();
		auto m = smd::g_alloc->New<smd::ShmMap<uint64_t, uint64_t>>();
		std::map<uint64_t, uint64_t> map_ref;

		std::vector<uint64_t> vRoleIds;
		const size_t COUNT = 1000;
		for (size_t i = 0; i < COUNT; i++) {
			vRoleIds.push_back(i);
		}

		std::random_shuffle(vRoleIds.begin(), vRoleIds.end());

		for (size_t i = 0; i < vRoleIds.size(); i++) {
			const auto& role_id = vRoleIds[i];
			auto value = role_id * 10;

			m->insert(smd::make_pair(role_id, value));
			map_ref.insert(std::make_pair(role_id, value));

			assert(m->IsEqual(map_ref));
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
};
