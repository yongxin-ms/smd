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
		fd_ = ::shm_open(
			name_.c_str(), O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

		if (mode == kCreateAlways) {
			if (fd_ == 0) {
				::shm_unlink(name_.c_str());
			}

			fd_ = ::shm_open(name_.c_str(), O_CREAT | O_EXCL | O_RDWR,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

			if (fd_ < 0) {
				m_log.DoLog(Log::LogLevel::kError, "fail shm_open[%d]: %s\n", errno, name_.c_str());
				return nullptr;
			}

			size_ = calc_size(size);
			if (::ftruncate(fd_, static_cast<off_t>(size_)) != 0) {
				m_log.DoLog(Log::LogLevel::kError, "fail ftruncate[%d]: %s, size = %zd\n", errno,
					name_.c_str(), size_);
				return nullptr;
			}

		} else {
			if (fd_ < 0) {
				m_log.DoLog(Log::LogLevel::kError, "fail shm_open[%d]: %s\n", errno, name_.c_str());
				return nullptr;
			}

			struct stat st;
			if (::fstat(fd_, &st) != 0) {
				m_log.DoLog(Log::LogLevel::kError, "fail fstat[%d]: %s, size = %zd\n", errno,
					name_.c_str(), size_);
				return nullptr;
			}
			size_ = static_cast<std::size_t>(st.st_size);
			if ((size_ <= sizeof(info_t)) || (size_ % sizeof(info_t))) {
				m_log.DoLog(Log::LogLevel::kError, "fail to_mem: %s, invalid size = %zd\n",
					name_.c_str(), size_);
				return nullptr;
			}

			if (size_ < size) {
				m_log.DoLog(Log::LogLevel::kError, "fail to_mem: %s, invalid size = %zd\n",
					name_.c_str(), size_);
				return nullptr;
			}
		}

		mem_ = ::mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
		if (mem_ == MAP_FAILED) {
			m_log.DoLog(Log::LogLevel::kError, "fail mmap[%d]: %s, size = %zd\n", errno,
				name_.c_str(), size_);
			return nullptr;
		}

		acc_of(mem_, size_).fetch_add(1, std::memory_order_release);
		return mem_;
	}

	void release() {
		if (mem_ != nullptr && size_ > 0) {
			::munmap(mem_, size_);
		}
	}

private:
	Log& m_log;
	int fd_ = -1;
	void* mem_ = nullptr;
	std::size_t size_ = 0;
	std::string name_;
};
} // namespace smd
