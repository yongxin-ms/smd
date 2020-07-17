#pragma once

namespace smd {
using SMD_POINTER = unsigned long long;
const SMD_POINTER SMD_NULL_PTR = 0;

enum : unsigned {
	create = 0x01,
	open = 0x02,
};

enum {
	MAGIC_NUM = 0x12345678,
	SERIAL_SIZE = 20 * 1024,
	SHM_MIN_SIZE = SERIAL_SIZE + 128 * 1024,
};

struct ShmHead {
	ShmHead() {
		memset(guid, 0, sizeof(guid));
		total_size = 0;
		create_time = 0;
		visit_num = 0;
		magic_num = 0;
		memset(reserve, 0, sizeof(reserve));
		len = 0;
		memset(data, 0, sizeof(data));
	}

	char guid[16];
	size_t total_size;
	time_t create_time;
	uint32_t visit_num;
	uint32_t magic_num;
	char reserve[256];
	size_t len;
	char data[1];
};

} // namespace smd
