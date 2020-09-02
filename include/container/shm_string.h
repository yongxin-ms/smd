#pragma once
#include <string>
#include <assert.h>
#include "../common/utility.h"
#include "../container/functional.h"
#include "../mem_alloc/alloc.h"
#include "shm_pointer.h"

namespace smd {

class ShmString {
public:
	ShmString(size_t size = 0) {
		resize(GetSuitableCapacity(size + 1));
	}

	ShmString(const std::string& r) {
		resize(GetSuitableCapacity(r.size() + 1));
		internal_copy(r.data(), r.size());
	}

	ShmString(const ShmString& r) {
		resize(GetSuitableCapacity(r.size() + 1));
		internal_copy(r.data(), r.size());
	}

	ShmString& operator=(const std::string& r) {
		ShmString(r).swap(*this);
		return *this;
	}

	ShmString& operator=(const ShmString& r) {
		if (this != &r) {
			ShmString(r).swap(*this);
		}
		return *this;
	}

	~ShmString() {
		resize(0);
		m_size = 0;
	}

	char* data() { return m_ptr.Ptr(); }
	const char* data() const { return m_ptr.Ptr(); }
	size_t size() const { return m_size; }
	bool empty() { return m_size > 0; }
	size_t capacity() const { return m_capacity; }

	ShmString& assign(const std::string& r) {
		if (r.size() < m_capacity) {
			internal_copy(r.data(), r.size());
			shrink_to_fit();
		} else {
			resize(GetSuitableCapacity(r.size() + 1));
			internal_copy(r.data(), r.size());
		}

		return *this;
	}

	ShmString& append(const ShmString& str) {
		if (capacity() <= size() + str.size()) {
			resize(GetSuitableCapacity(size() + str.size() + 1));
		}

		internal_append(str.data(), str.size());
		return *this;
	}

	ShmString& append(const std::string& str) {
		if (capacity() <= size() + str.size()) {
			resize(GetSuitableCapacity(size() + str.size() + 1));
		}

		internal_append(str.data(), str.size());
		return *this;
	}

	ShmString& append(const char* s) {
		size_t len = strlen(s);
		if (capacity() <= size() + len) {
			resize(GetSuitableCapacity(size() + len + 1));
		}

		internal_append(s, len);
		return *this;
	}

	ShmString& append(const char* s, size_t n) {
		if (capacity() <= size() + n) {
			resize(GetSuitableCapacity(size() + n + 1));
		}

		internal_append(s, n);
		return *this;
	}

	int compare(const ShmString& b) const {
		const size_t min_len = (size() < b.size()) ? size() : b.size();
		int r = memcmp(data(), b.data(), min_len);
		if (r == 0) {
			if (size() < b.size())
				r = -1;
			else if (size() > b.size())
				r = +1;
		}
		return r;
	}

	void clear() {
		m_size = 0;
		shrink_to_fit();
	}

	std::string ToString() const { return std::string(data(), size()); }

	bool operator==(const ShmString& rhs) const {
		auto p1 = data();
		auto p2 = rhs.data();
		return ((size() == rhs.size()) && (memcmp(p1, p2, size()) == 0));
	}

	//测试专用
	bool IsEqual(const std::string& stl_str) {
		if (size() != stl_str.size()) {
			assert(false);
		}

		auto& str = ToString();
		if (str != stl_str) {
			assert(false);
		}

		return true;
	}

private:
	static size_t GetSuitableCapacity(size_t size) {
		if (size <= 16)
			return 16;
		else
			return Utility::NextPowOf2(size);
	}

	void swap(ShmString& x) {
		std::swap(m_ptr, x.m_ptr);
		std::swap(m_capacity, x.m_capacity);
		std::swap(m_size, x.m_size);
	}

	void resize(size_t capacity) {
		if (m_ptr != shm_nullptr) {
			g_alloc->Free(m_ptr, m_capacity);
			m_capacity = 0;
		}

		if (capacity > 0) {
			m_capacity = capacity;
			m_ptr = g_alloc->Malloc<char>(m_capacity);
		}
	}

	void internal_copy(const char* buf, size_t len) {
		m_size = 0;
		internal_append(buf, len);
	}

	void internal_append(const char* buf, size_t len) {
		// 最后有一个0
		assert(m_capacity > m_size + len);
		char* ptr = m_ptr.Ptr();
		memcpy(&ptr[m_size], buf, len);
		ptr[len] = '\0';
		m_size = len;
	}

	void shrink_to_fit() {
		// 在此添加代码缩容
	}

private:
	ShmPointer<char> m_ptr = shm_nullptr;
	size_t m_capacity = 0;
	size_t m_size = 0;
};

inline bool operator!=(const ShmString& x, const ShmString& y) { return !(x == y); }
inline bool operator<(const ShmString& x, const ShmString& y) { return x.compare(y) < 0; }
inline bool operator>(const ShmString& x, const ShmString& y) { return x.compare(y) > 0; }

} // namespace smd

namespace std {
template <>
struct hash<smd::ShmString> {
	typedef smd::ShmString argument_type;
	typedef std::size_t result_type;

	result_type operator()(argument_type const& s) const {
		return std::hash<std::string>()(s.ToString());
	}
};

} // namespace std
