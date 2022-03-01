#pragma once
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <atomic>
#include <string>
#include <utility>
#include <cstring>

#include <common/log.h>
#include <container/shm_defines.h>

namespace smd {
struct info_t {
	std::atomic_size_t acc_;
};

constexpr std::size_t calc_size(std::size_t size) {
	return ((((size - 1) / alignof(info_t)) + 1) * alignof(info_t)) + sizeof(info_t);
}

inline std::atomic_size_t& acc_of(void* mem, std::size_t size) {
	return reinterpret_cast<info_t*>(static_cast<unsigned char*>(mem) + size - sizeof(info_t))->acc_;
}

class ShmLinux {
public:
	void* acquire(int shm_key, size_t size, ShareMemOpenMode mode) {
		size_ = calc_size(size);
		shm_id_ = shmget(shm_key, size_, 0);

		if (mode == ShareMemOpenMode::kCreateAlways) {
			if (shm_id_ > 0) {
				if (shmctl(shm_id_, IPC_RMID, nullptr) < 0) {
					SMD_LOG_ERROR("fail shmctl IPC_RMID[%08x]: %d\n", shm_key, errno);
					return nullptr;
				}
			}

			shm_id_ = shmget(shm_key, size_, 0640 | IPC_CREAT | IPC_EXCL);
			if (shm_id_ < 0) {
				SMD_LOG_ERROR("fail shmget IPC_EXCL [%08x]: %d\n", shm_key, errno);
				return nullptr;
			}
		} else {
			if (shm_id_ < 0) {
				SMD_LOG_ERROR("fail shmget IPC_CREAT [%08x]: %d\n", shm_key, errno);
				return nullptr;
			}
		}

		mem_ = shmat(shm_id_, nullptr, 0);
		if (mem_ == reinterpret_cast<void*>(-1)) {
			SMD_LOG_ERROR("fail shmat[%08x]: %d, size = %zd\n", shm_key, errno, size_);
			return nullptr;
		}

		acc_of(mem_, size_).fetch_add(1, std::memory_order_release);
		return mem_;
	}

	void release() {
		if (mem_ != nullptr && size_ > 0) {
			shmdt(mem_);
			mem_ = nullptr;
			size_ = 0;
		}
	}

private:
	int shm_id_ = -1;
	void* mem_ = nullptr;
	size_t size_ = 0;
};
} // namespace smd
