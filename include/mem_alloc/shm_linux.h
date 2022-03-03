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
	std::pair<void*, bool> acquire(int shm_key, size_t size, bool enable_attach) {
		size_ = calc_size(size);
		auto shm_id = shmget(shm_key, size_, 0);
		bool is_attached = true;

		if (!enable_attach) {
			if (shm_id > 0) {
				if (shmctl(shm_id, IPC_RMID, nullptr) < 0) {
					SMD_LOG_ERROR("fail shmctl IPC_RMID[%08x]: %d\n", shm_key, errno);
					return std::make_pair(nullptr, is_attached);
				}
			}

			shm_id = shmget(shm_key, size_, 0640 | IPC_CREAT | IPC_EXCL);
			if (shm_id < 0) {
				SMD_LOG_ERROR("fail shmget IPC_EXCL [%08x]: %d\n", shm_key, errno);
				return std::make_pair(nullptr, is_attached);
			}

			is_attached = false;
		} else {
			if (shm_id < 0) {
				shm_id = shmget(shm_key, size_, 0640 | IPC_CREAT | IPC_EXCL);
			}
			
			if (shm_id < 0) {
				SMD_LOG_ERROR("fail shmget IPC_CREAT [%08x]: %d\n", shm_key, errno);
				return std::make_pair(nullptr, is_attached);
			}
		}

		mem_ = shmat(shm_id, nullptr, 0);
		if (mem_ == reinterpret_cast<void*>(-1)) {
			SMD_LOG_ERROR("fail shmat[%08x]: %d, size = %zd\n", shm_key, errno, size_);
			return std::make_pair(nullptr, is_attached);
		}

		acc_of(mem_, size_).fetch_add(1, std::memory_order_release);
		return std::make_pair(mem_, is_attached);
	}

	void release() {
		if (mem_ != nullptr && size_ > 0) {
			shmdt(mem_);
			mem_ = nullptr;
			size_ = 0;
		}
	}

private:
	void* mem_ = nullptr;
	size_t size_ = 0;
};
} // namespace smd
