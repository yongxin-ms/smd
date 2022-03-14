#pragma once
#include <sm_env.h>

namespace smd {

struct StSmd {
	shm_map<shm_string, shm_string> all_strings;
	shm_map<shm_string, shm_list<shm_string>> all_lists;
	shm_map<shm_string, shm_map<shm_string, shm_string>> all_maps;
	shm_map<shm_string, shm_hash<shm_string>> all_hashes;
};

class SmdEnv : public smd::Env<StSmd> {
public:
	//
	// 内置string, list, map, hash 四种基本数据类型
	//
	shm_map<shm_string, shm_string>& GetAllStrings() {
		return GetEntry().all_strings;
	}

	shm_map<shm_string, shm_list<shm_string>>& GetAllLists() {
		return GetEntry().all_lists;
	}

	shm_map<shm_string, shm_map<shm_string, shm_string>>& GetAllMaps() {
		return GetEntry().all_maps;
	}

	shm_map<shm_string, shm_hash<shm_string>>& GetAllHashes() {
		return GetEntry().all_hashes;
	}

	//
	// 字符串操作
	//
	// 写操作
	void SSet(const Slice& key, const Slice& value);
	// 读操作
	bool SGet(const Slice& key, Slice* value);
	// 删除操作
	bool SDel(const Slice& key);
};

// 写操作
void SmdEnv::SSet(const Slice& key, const Slice& value) {
	auto& all_strings = GetAllStrings();
	shm_string str_key(key.data(), key.size());
	auto it = all_strings.find(str_key);
	if (it == all_strings.end()) {
		shm_string str_value(value.data(), value.size());
		all_strings.insert(std::make_pair(str_key, str_value));
	} else {
		it->second = value.ToString();
	}
}

// 读操作
bool SmdEnv::SGet(const Slice& key, Slice* value) {
	auto& all_strings = GetAllStrings();
	shm_string str_value(key.data(), key.size());
	auto it = all_strings.find(str_value);
	if (it == all_strings.end()) {
		return false;
	} else {
		if (value != nullptr) {
			const auto& str_value = it->second;

			// 返回的是数据指针，调用者自己决定是否需要拷贝
			*value = Slice(str_value.data(), str_value.size());
		}

		return true;
	}
}

// 删除操作
bool SmdEnv::SDel(const Slice& key) {
	auto& all_strings = GetAllStrings();
	shm_string str_value(key.data(), key.size());
	auto it = all_strings.find(str_value);
	if (it == all_strings.end()) {
		return false;
	}

	it = all_strings.erase(it);
	return true;
}
} // namespace smd
