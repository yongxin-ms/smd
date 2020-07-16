#include <windows.h>
#include <string>
#include "../common/smd_defines.h"
#include "../common/log.h"

namespace smd {

class ShmWin {
public:
	ShmWin(Log& log)
		: m_log(log) {}

	bool acquire(const std::string& fmt_name, std::size_t size, unsigned mode) {
		if (fmt_name.size() <= 0) {
			m_log.DoLog(Log::LogLevel::kError, "fail acquire: name is empty\n");
			return false;
		}

		HANDLE h;
		if (mode == open) {
			h = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, fmt_name.c_str());
		} else {
			h = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0,
				static_cast<DWORD>(size), fmt_name.c_str());
			if ((mode == create) && (::GetLastError() == ERROR_ALREADY_EXISTS)) {
				::CloseHandle(h);
				h = NULL;
			}
		}

		if (h == NULL) {
			m_log.DoLog(Log::LogLevel::kError, "fail CreateFileMapping/OpenFileMapping[%d]: %s\n",
				static_cast<int>(::GetLastError()), fmt_name.data());
			return false;
		}

		h_ = h;
		size_ = size;
		return true;
	}

	void* get_mem(std::size_t* size) {
		if (mem_ != nullptr) {
			if (size != nullptr)
				*size = size_;
			return mem_;
		}

		if (h_ == NULL) {
			m_log.DoLog(Log::LogLevel::kError, "fail to_mem: invalid id (h = null)\n");
			return nullptr;
		}

		LPVOID mem = ::MapViewOfFile(h_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (mem == NULL) {
			m_log.DoLog(Log::LogLevel::kError, "fail MapViewOfFile[%d]\n",
				static_cast<int>(::GetLastError()));
			return nullptr;
		}

		MEMORY_BASIC_INFORMATION mem_info;
		if (::VirtualQuery(mem, &mem_info, sizeof(mem_info)) == 0) {
			m_log.DoLog(Log::LogLevel::kError, "fail VirtualQuery[%d]\n",
				static_cast<int>(::GetLastError()));
			return nullptr;
		}

		mem_ = mem;
		size_ = static_cast<std::size_t>(mem_info.RegionSize);
		if (size != nullptr)
			*size = size_;
		return static_cast<void*>(mem);
	}

	void release() {
		if (mem_ == nullptr || size_ == 0) {
			m_log.DoLog(Log::LogLevel::kError, "fail release: invalid id (mem = %p, size = %zd)\n",
				mem_, size_);
		} else {
			::UnmapViewOfFile(static_cast<LPCVOID>(mem_));
		}

		if (h_ == NULL) {
			m_log.DoLog(Log::LogLevel::kError, "fail release: invalid id (h = null)\n");
		} else {
			::CloseHandle(h_);
		}
	}

	void remove() {}
	void remove(const std::string& name) {}

private:
	Log& m_log;

	HANDLE h_ = NULL;
	std::size_t size_ = 0;
	void* mem_ = nullptr;
};

} // namespace smd
