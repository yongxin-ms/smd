﻿#pragma once
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

namespace smd {
class SmdBuddyAlloc {
public:
	enum : uint8_t {
		NODE_UNUSED = 0,
		NODE_USED = 1,
		NODE_SPLIT = 2,
		NODE_FULL = 3,
	};

	enum {
		MAX_LEVEL = 32,
	};

#pragma pack(push, 1)
	struct buddy {
		int level;
		uint8_t tree[1];
	};
#pragma pack(pop)

	static int get_index_size(int level) {
		int size = 1 << level;
		return (sizeof(buddy) + sizeof(uint8_t) * (size * 2 - 2));
	}

	static int get_storage_size(int level) {
		int size = 1 << (level + 1);
		return size;
	}

	static buddy* buddy_new(const char* p, int level) {
		int size = 1 << level;

		buddy* self = (buddy*)p;
		self->level = level;
		memset(self->tree, NODE_UNUSED, size * 2 - 1);
		return self;
	}

	static int buddy_alloc(buddy* self, uint32_t s) {
		const uint32_t size = s == 0 ? 1 : next_pow_of_2(s);
		uint32_t length = 1 << self->level;

		// 空间不够了
		if (size > length)
			return -1;

		int index = 0;
		int level = 0;

		while (index >= 0) {
			if (size == length) {
				if (self->tree[index] == NODE_UNUSED) {
					self->tree[index] = NODE_USED;
					_mark_parent(self, index);
					return _index_offset(index, level, self->level);
				}
			} else {
				// size < length
				switch (self->tree[index]) {
				case NODE_USED:
				case NODE_FULL:
					break;
				case NODE_UNUSED:
					// split first
					self->tree[index] = NODE_SPLIT;
					self->tree[index * 2 + 1] = NODE_UNUSED;
					self->tree[index * 2 + 2] = NODE_UNUSED;
				default:
					index = index * 2 + 1;
					length /= 2;
					level++;
					continue;
				}
			}
			if (index & 1) {
				++index;
				continue;
			}
			for (;;) {
				level--;
				length *= 2;
				index = (index + 1) / 2 - 1;
				if (index < 0)
					return -1;
				if (index & 1) {
					++index;
					break;
				}
			}
		}

		return -1;
	}

	static void buddy_free(buddy* self, int offset) {
		assert(offset < (1 << self->level));
		int left = 0;
		int length = 1 << self->level;
		int index = 0;

		for (;;) {
			switch (self->tree[index]) {
			case NODE_USED:
				assert(offset == left);
				_combine(self, index);
				return;
			case NODE_UNUSED:
				assert(0);
				return;
			default:
				length /= 2;
				if (offset < left + length) {
					index = index * 2 + 1;
				} else {
					left += length;
					index = index * 2 + 2;
				}
				break;
			}
		}
	}

	static int buddy_size(buddy* self, int offset) {
		assert(offset < (1 << self->level));
		int left = 0;
		int length = 1 << self->level;
		int index = 0;

		for (;;) {
			switch (self->tree[index]) {
			case NODE_USED:
				assert(offset == left);
				return length;
			case NODE_UNUSED:
				assert(0);
				return length;
			default:
				length /= 2;
				if (offset < left + length) {
					index = index * 2 + 1;
				} else {
					left += length;
					index = index * 2 + 2;
				}
				break;
			}
		}
	}

	static void _dump(buddy* self, int index, int level) {
		switch (self->tree[index]) {
		case NODE_UNUSED:
			printf("(%d:%d)", _index_offset(index, level, self->level), 1 << (self->level - level));
			break;
		case NODE_USED:
			printf("[%d:%d]", _index_offset(index, level, self->level), 1 << (self->level - level));
			break;
		case NODE_FULL:
			printf("{");
			_dump(self, index * 2 + 1, level + 1);
			_dump(self, index * 2 + 2, level + 1);
			printf("}");
			break;
		default:
			printf("(");
			_dump(self, index * 2 + 1, level + 1);
			_dump(self, index * 2 + 2, level + 1);
			printf(")");
			break;
		}
	}

	void buddy_dump(buddy* self) {
		_dump(self, 0, 0);
		printf("\n");
	}

private:
	static inline uint32_t is_pow_of_2(uint32_t x) {
		return !(x & (x - 1));
	}

	static inline uint32_t next_pow_of_2(uint32_t x) {
		if (is_pow_of_2(x))
			return x;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		return x + 1;
	}

	static inline int _index_offset(int index, int level, int max_level) {
		return ((index + 1) - (1 << level)) << (max_level - level);
	}

	static void _mark_parent(buddy* self, int index) {
		for (;;) {
			int buddy = index - 1 + (index & 1) * 2;
			if (buddy > 0 && (self->tree[buddy] == NODE_USED || self->tree[buddy] == NODE_FULL)) {
				index = (index + 1) / 2 - 1;
				self->tree[index] = NODE_FULL;
			} else {
				return;
			}
		}
	}

	static void _combine(buddy* self, int index) {
		for (;;) {
			int buddy = index - 1 + (index & 1) * 2;
			if (buddy < 0 || self->tree[buddy] != NODE_UNUSED) {
				self->tree[index] = NODE_UNUSED;
				while (((index = (index + 1) / 2 - 1) >= 0) && self->tree[index] == NODE_FULL) {
					self->tree[index] = NODE_SPLIT;
				}
				return;
			}
			index = (index + 1) / 2 - 1;
		}
	}
};

} // namespace smd
