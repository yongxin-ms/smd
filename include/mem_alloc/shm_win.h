#pragma once
#include <windows.h>
#include <string>

namespace smd {

class ShmWin {
public:
	ShmWin() {}

	std::pair<void*, bool> acquire(int shm_key, size_t size, bool enable_attach) {
		char fmt_name[64];
		_snprintf_s(fmt_name, sizeof(fmt_name), "%d", shm_key);

		DWORD dwMaximumSizeHigh = static_cast<DWORD>(size >> 32);
		DWORD dwMaximumSizeLow = static_cast<DWORD>(size);
		bool is_attached = true;
		if (enable_attach) {
			m_handle = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, fmt_name);
			if (m_handle == NULL) {
				SMD_LOG_ERROR("OpenFileMapping failed key:%s, errno:%u", fmt_name, ::GetLastError());
				m_handle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT,
											   dwMaximumSizeHigh, dwMaximumSizeLow, fmt_name);
				is_attached = false;
				if (m_handle == NULL) {
					SMD_LOG_ERROR("CreateFileMapping failed key:%s, errno:%u", fmt_name, ::GetLastError());
					return std::make_pair(nullptr, is_attached);
				}

				SMD_LOG_INFO("CreateFileMapping successfully key:%s, size:%llu", fmt_name, size);
			} else {
				SMD_LOG_INFO("OpenFileMapping successfully key:%s", fmt_name);
			}
		} else {
			m_handle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, dwMaximumSizeHigh,
										   dwMaximumSizeLow, fmt_name);
			is_attached = false;
			if (m_handle == NULL) {
				SMD_LOG_ERROR("CreateFileMapping failed key:%s, errno:%u", fmt_name, ::GetLastError());
				return std::make_pair(nullptr, is_attached);
			}

			SMD_LOG_INFO("CreateFileMapping successfully key:%s, size:%llu", fmt_name, size);
		}

		m_memPtr = ::MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (m_memPtr == nullptr) {
			SMD_LOG_ERROR("MapViewOfFile failed key:%s, errno:%u", fmt_name, ::GetLastError());
			return std::make_pair(nullptr, is_attached);
		}

		SMD_LOG_INFO("MapViewOfFile successfully key:%s", fmt_name);
		return std::make_pair(m_memPtr, is_attached);
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
