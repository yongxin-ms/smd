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

#include "../common/log.h"
#include "../common/shm_defines.h"

namespace {

struct info_t {
	std::atomic_size_t acc_;
};

constexpr std::size_t calc_size(std::size_t size) {
	return ((((size - 1) / alignof(info_t)) + 1) * alignof(info_t)) + sizeof(info_t);
}

inline std::atomic_size_t& acc_of(void* mem, std::size_t size) {
	return reinterpret_cast<info_t*>(static_cast<unsigned char*>(mem) + size - sizeof(info_t))->acc_;
}

} // namespace

namespace smd {
class ShmLinux {
public:
	ShmLinux(Log& log)
		: m_log(log) {}

	void* acquire(const std::string& fmt_name, std::size_t size, ShareMemOpenMode mode) {
		name_ = fmt_name;
		size_ = calc_size(size);
		shm_id_ = shmget(fmt_name, size, 0640 | IPC_CREAT);

		if (mode == kCreateAlways) {
			if (shm_id_ == 0) {
				if (shmctl(shm_id, IPC_RMID, nullptr) < 0) {
					m_log.DoLog(
						Log::LogLevel::kError, "fail shmctl IPC_RMID[%d]: %s\n", errno, name_.c_str());
					return nullptr;
				}
			}

			shm_id_ = shmget(fmt_name, size_, 0640 | IPC_CREAT | IPC_EXCL);
			if (shm_id_ < 0) {
				m_log.DoLog(Log::LogLevel::kError, "fail shmget IPC_EXCL [%d]: %s\n", errno, name_.c_str());
				return nullptr;
			}
		} else {
			if (shm_id_ < 0) {
				m_log.DoLog(Log::LogLevel::kError, "fail shmget IPC_CREAT [%d]: %s\n", errno, name_.c_str());
				return nullptr;
			}
		}

		mem_ = shmat(shm_id_, nullptr, 0);
		if (mem_ == reinterpret_cast<void*>(-1)) {
			m_log.DoLog(Log::LogLevel::kError, "fail shmat[%d]: %s, size = %zd\n", errno,
				name_.c_str(), size_);
			return nullptr;
		}

		acc_of(mem_, size_).fetch_add(1, std::memory_order_release);
		return mem_;
	}

	void release() {
		if (mem_ != nullptr && size_ > 0) {
			shmdt(mem_);
		}
	}

private:
	Log& m_log;
	int shm_id_ = -1;
	void* mem_ = nullptr;
	std::size_t size_ = 0;
	std::string name_;
};
} // namespace smd
