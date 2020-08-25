#pragma once
#include <windows.h>
#include <string>
#include "../common/shm_defines.h"
#include "../common/log.h"

namespace smd {

class ShmWin {
public:
	ShmWin(Log& log)
		: m_log(log) {}

	bool acquire(const std::string& fmt_name, std::size_t size, ShareMemOpenMode mode) {
		if (fmt_name.size() <= 0) {
			m_log.DoLog(Log::LogLevel::kError, "fail acquire: name is empty\n");
			return false;
		}

		HANDLE h;
		if (mode == kOpenExist) {
			h = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, fmt_name.c_str());
			if (h == NULL) {
				h = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0,
					static_cast<DWORD>(size), fmt_name.c_str());
			}
		} else {
			h = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0,
				static_cast<DWORD>(size), fmt_name.c_str());
// 			if ((::GetLastError() == ERROR_ALREADY_EXISTS)) {
// 				::CloseHandle(h);
// 				h = NULL;
// 			}
		}

		if (h == NULL) {
			m_log.DoLog(Log::LogLevel::kError, "fail CreateFileMapping/OpenFileMapping[%d]: %s\n",
				static_cast<int>(::GetLastError()), fmt_name.data());
			return false;
		}

		m_handle = h;
		m_size = size;
		return true;
	}

	void* get_mem(std::size_t* size) {
		if (m_memPtr != nullptr) {
			if (size != nullptr)
				*size = m_size;
			return m_memPtr;
		}

		if (m_handle == NULL) {
			m_log.DoLog(Log::LogLevel::kError, "fail to_mem: invalid id (h = null)\n");
			return nullptr;
		}

		LPVOID mem = ::MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (mem == nullptr) {
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

		m_memPtr = mem;
		m_size = static_cast<std::size_t>(mem_info.RegionSize);
		if (size != nullptr)
			*size = m_size;
		return static_cast<void*>(mem);
	}

	void release() {
		if (m_memPtr == nullptr || m_size == 0) {
			m_log.DoLog(Log::LogLevel::kError, "fail release: invalid id (mem = %p, size = %zd)\n",
				m_memPtr, m_size);
		} else {
			::UnmapViewOfFile(static_cast<LPCVOID>(m_memPtr));
		}

		if (m_handle == NULL) {
			m_log.DoLog(Log::LogLevel::kError, "fail release: invalid id (h = null)\n");
		} else {
			::CloseHandle(m_handle);
		}
	}

	void remove() {}
	void remove(const std::string& name) {}

private:
	Log& m_log;

	HANDLE m_handle = NULL;
	std::size_t m_size = 0;
	void* m_memPtr = nullptr;
};

} // namespace smd
