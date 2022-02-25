#pragma once
#include <windows.h>
#include <string>
#include <container/shm_defines.h>

namespace smd {

class ShmWin {
public:
	ShmWin() {}

	void* acquire(int shm_key, std::size_t size, ShareMemOpenMode mode) {
		char fmt_name[64];
		_snprintf_s(fmt_name, sizeof(fmt_name), "%08x", shm_key);
		if (mode == kOpenExist) {
			m_handle = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, fmt_name);
			if (m_handle == NULL) {
				m_handle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0,
											   static_cast<DWORD>(size), fmt_name);
			}
		} else {
			m_handle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0,
										   static_cast<DWORD>(size), fmt_name);
			// 			if ((::GetLastError() == ERROR_ALREADY_EXISTS)) {
			// 				::CloseHandle(h);
			// 				m_handle = NULL;
			// 			}
		}

		if (m_handle == NULL) {
			SMD_LOG_ERROR("fail CreateFileMapping/OpenFileMapping[%d]: %s\n", static_cast<int>(::GetLastError()),
						  fmt_name);
			return nullptr;
		}

		m_memPtr = ::MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (m_memPtr == nullptr) {
			SMD_LOG_ERROR("fail MapViewOfFile[%d]\n", static_cast<int>(::GetLastError()));
			return nullptr;
		}

		m_size = size;
		return m_memPtr;
	}

	void release() {
		if (m_memPtr != nullptr) {
			::UnmapViewOfFile(static_cast<LPCVOID>(m_memPtr));
			m_memPtr = nullptr;
		}

		if (m_handle != NULL) {
			::CloseHandle(m_handle);
			m_handle = NULL;
		}
	}

private:
	HANDLE m_handle = NULL;
	std::size_t m_size = 0;
	void* m_memPtr = nullptr;
};

} // namespace smd
