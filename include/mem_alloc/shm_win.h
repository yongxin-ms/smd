#pragma once
#include <windows.h>
#include <string>
#include <container/shm_defines.h>

namespace smd {

class ShmWin {
public:
	ShmWin() {}

	std::pair<void*, bool> acquire(int shm_key, size_t size, bool enable_attach) {
		char fmt_name[64];
		_snprintf_s(fmt_name, sizeof(fmt_name), "%08x", shm_key);

		DWORD dwMaximumSizeHigh = static_cast<DWORD>(size >> 32);
		DWORD dwMaximumSizeLow = static_cast<DWORD>(size);
		bool attached = true;
		if (enable_attach) {
			m_handle = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, fmt_name);
			if (m_handle == NULL) {
				m_handle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT,
											   dwMaximumSizeHigh, dwMaximumSizeLow, fmt_name);
				attached = false;
			}
		} else {
			m_handle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, dwMaximumSizeHigh,
										   dwMaximumSizeLow, fmt_name);
			attached = false;
		}

		if (m_handle == NULL) {
			SMD_LOG_ERROR("fail CreateFileMapping/OpenFileMapping[%u]: %s\n", ::GetLastError(), fmt_name);
			return std::make_pair(nullptr, attached);
		}

		m_memPtr = ::MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (m_memPtr == nullptr) {
			SMD_LOG_ERROR("fail MapViewOfFile[%u]\n", ::GetLastError());
			return std::make_pair(nullptr, attached);
		}

		return std::make_pair(m_memPtr, attached);
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
	void* m_memPtr = nullptr;
};

} // namespace smd
