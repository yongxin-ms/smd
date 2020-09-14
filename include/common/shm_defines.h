#pragma once

namespace smd {

// 共享内存的两种开启模式
enum ShareMemOpenMode {
	// 总是创建新的，不管之前是否存在
	// 如果之前有数据，会覆盖掉已有的数据
	kCreateAlways = 1,

	// 打开已存在的，如果不存在则创建新的
	kOpenExist,
};

enum {
	GUID_SIZE = 15,
	MAGIC_NUM = 0x12345678,
};

class ShmString;

template <class T>
class ShmList;

template <typename K, typename V>
class ShmMap;

template <class T>
class ShmHash;

template <typename T>
class ShmPointer;

#pragma pack(push, 1)
struct StGlobalVariable {
	ShmPointer<ShmMap<ShmString, ShmString>> allStrings;
	ShmPointer<ShmMap<ShmString, ShmList<ShmString>>> allLists;
	ShmPointer<ShmMap<ShmString, ShmMap<ShmString, ShmString>>> allMaps;
	ShmPointer<ShmMap<ShmString, ShmHash<ShmString>>> allHashes;

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
