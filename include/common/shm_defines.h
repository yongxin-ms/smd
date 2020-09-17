#pragma once

namespace smd {

//
// 共享内存的两种开启模式
//
enum ShareMemOpenMode {

	// 冷重启，总是创建新的，不管之前是否存在
	// 如果之前有数据，会覆盖掉已有的数据
	kCreateAlways = 1,

	// 热重启，打开已存在的，如果不存在则失败
	kOpenExist,
};

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

	char reserve[256];
};

struct ShmHead {
	char guid[GUID_SIZE + 1];
	size_t total_size;
	time_t create_time;
	time_t last_visit_time;
	uint32_t visit_num;
	uint32_t magic_num;
	StGlobalVariable global_variable;
};
#pragma pack(pop)

} // namespace smd
