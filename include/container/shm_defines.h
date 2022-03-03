#pragma once

namespace smd {
enum {
	GUID_SIZE = 15,
	MAGIC_NUM = 0x12345678,
};

class shm_string;

template <class T>
class shm_list;

template <typename K, typename V>
class shm_map;

template <class T>
class shm_hash;

template <typename T>
class shm_pointer;

#pragma pack(push, 1)
struct StGlobalVariable {
	shm_pointer<shm_map<shm_string, shm_string>> allStrings;
	shm_pointer<shm_map<shm_string, shm_list<shm_string>>> allLists;
	shm_pointer<shm_map<shm_string, shm_map<shm_string, shm_string>>> allMaps;
	shm_pointer<shm_map<shm_string, shm_hash<shm_string>>> allHashes;
};

struct ShmHead {
	size_t total_size;
	time_t create_time;
	time_t last_visit_time;
	uint32_t visit_num;
	uint32_t magic_num;
	StGlobalVariable global_variable;
};
#pragma pack(pop)

} // namespace smd
