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

	bool acquire(const std::string& fmt_name, std::size_t size, ShareMemOpenMode mode) {
		int fd = ::shm_open(
			fmt_name.c_str(), O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

		if (mode == kCreateAlways) {
			if (fd == -1) {
				fd = ::shm_open(fmt_name.c_str(), O_CREAT | O_EXCL | O_RDWR,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			}
		} else {
			size = 0;
		}

		if (fd == -1) {
			m_log.DoLog(Log::LogLevel::kError, "fail shm_open[%d]: %s\n", errno, fmt_name.c_str());
			return false;
		}

		fd_ = fd;
		size_ = size;
		name_ = fmt_name;
		return true;
	}

	void* get_mem(std::size_t* size) {
		if (mem_ != nullptr) {
			if (size != nullptr)
				*size = size_;
			return mem_;
		}

		int fd = fd_;
		if (fd == -1) {
			m_log.DoLog(Log::LogLevel::kError, "fail to_mem: invalid id (fd = -1)\n");
			return nullptr;
		}

		if (size_ == 0) {
			struct stat st;
			if (::fstat(fd, &st) != 0) {
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
		} else {
			size_ = calc_size(size_);
			if (::ftruncate(fd, static_cast<off_t>(size_)) != 0) {
				m_log.DoLog(Log::LogLevel::kError, "fail ftruncate[%d]: %s, size = %zd\n", errno,
					name_.c_str(), size_);
				return nullptr;
			}
		}

		void* mem = ::mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (mem == MAP_FAILED) {
			m_log.DoLog(Log::LogLevel::kError, "fail mmap[%d]: %s, size = %zd\n", errno,
				name_.c_str(), size_);
			return nullptr;
		}

		::close(fd);
		fd_ = -1;
		mem_ = mem;
		if (size != nullptr)
			*size = size_;
		acc_of(mem, size_).fetch_add(1, std::memory_order_release);
		return mem;
	}

	void release() {
		if (mem_ == nullptr || size_ == 0) {
			m_log.DoLog(Log::LogLevel::kError, "fail release: invalid id (mem = %p, size = %zd)\n",
				mem_, size_);
		} else if (acc_of(mem_, size_).fetch_sub(1, std::memory_order_acquire) == 1) {
			::munmap(mem_, size_);
			if (!name_.empty()) {
				::shm_unlink(name_.c_str());
			}
		} else {
			::munmap(mem_, size_);
		}
	}

	void remove() {
		auto name = name_;
		release();
		if (!name.empty()) {
			::shm_unlink(name.c_str());
		}
	}

	void remove(const std::string& name) {
		if (name.size() <= 0) {
			m_log.DoLog(Log::LogLevel::kError, "fail remove: name is empty\n");
			return;
		}

		::shm_unlink(name.c_str());
	}

private:
	Log& m_log;
	int fd_ = -1;
	void* mem_ = nullptr;
	std::size_t size_ = 0;
	std::string name_;
};
} // namespace smd
