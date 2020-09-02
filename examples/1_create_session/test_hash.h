#pragma once
#include "smd.h"

class TestHash {
public:
	TestHash(smd::Log& log) {
		TestHashBasic(log);
		TestHashPod(log);
	}

private:
	void TestHashBasic(smd::Log& log) {
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

		smd::g_alloc->Delete(h_ref);
		assert(h_ref == smd::shm_nullptr);
		assert(mem_usage == smd::g_alloc->GetUsed());

		log.DoLog(smd::Log::LogLevel::kInfo, "TestHashPod complete");
	}
};
