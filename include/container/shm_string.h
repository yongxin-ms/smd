#pragma once
#include "reverse_iterator.h"
#include "pair.h"
#include "../mem_alloc/alloc.h"

namespace smd {

class ShmString {
public:
	typedef char value_type;
	typedef char* iterator;
	typedef const char* const_iterator;
	typedef reverse_iterator_t<char*> reverse_iterator;
	typedef reverse_iterator_t<const char*> const_reverse_iterator;
	typedef char& reference;
	typedef const char& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	// npos is a static member constant value with the greatest possible value for an element of
	// type size_t.
	static const size_t npos = -1;

public:
	ShmString()
		: m_start(nullptr)
		, m_finish(nullptr)
		, m_endOfStorage(nullptr) {}
	ShmString(const ShmString& str);
	ShmString(ShmString&& str);
	ShmString(const ShmString& str, size_t pos, size_t len = npos);
	ShmString(const char* s);
	ShmString(const char* s, size_t n);
	ShmString(size_t n, char c);
	template <class InputIterator>
	ShmString(InputIterator first, InputIterator last);

	ShmString& operator=(const ShmString& str);
	ShmString& operator=(ShmString&& str);
	ShmString& operator=(const char* s);
	ShmString& operator=(char c);

	~ShmString();

	iterator begin() { return m_start; }
	const_iterator begin() const { return m_start; }
	iterator end() { return m_finish; }
	const_iterator end() const { return m_finish; }
	reverse_iterator rbegin() { return reverse_iterator(m_finish); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(m_finish); }
	reverse_iterator rend() { return reverse_iterator(m_start); }
	const_reverse_iterator rend() const { return const_reverse_iterator(m_start); }
	const_iterator cbegin() const { return m_start; }
	const_iterator cend() const { return m_finish; }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(m_finish); }
	const_reverse_iterator crend() const { return const_reverse_iterator(m_start); }
	size_t size() const { return m_finish - m_start; }
	size_t length() const { return size(); }
	size_t capacity() const { return m_endOfStorage - m_start; }
	void clear() {
		dataAllocator::destroy(m_start, m_finish);
		m_start = m_finish;
	}
	bool empty() const { return begin() == end(); }
	void resize(size_t n);
	void resize(size_t n, char c);
	void reserve(size_t n = 0);
	void shrink_to_fit() {
		dataAllocator::deallocate(m_finish, m_endOfStorage - m_finish);
		m_endOfStorage = m_finish;
	}

	char& operator[](size_t pos) { return *(m_start + pos); }
	const char& operator[](size_t pos) const { return *(m_start + pos); }
	char& back() { return *(m_finish - 1); }
	const char& back() const { return *(m_finish - 1); }
	char& front() { return *(m_start); }
	const char& front() const { return *(m_start); }

	void push_back(char c) { insert(end(), c); }
	ShmString& insert(size_t pos, const ShmString& str);
	ShmString& insert(size_t pos, const ShmString& str, size_t subpos, size_t sublen = npos);
	ShmString& insert(size_t pos, const char* s);
	ShmString& insert(size_t pos, const char* s, size_t n);
	ShmString& insert(size_t pos, size_t n, char c);
	iterator insert(iterator p, size_t n, char c);
	iterator insert(iterator p, char c);
	template <class InputIterator>
	iterator insert(iterator p, InputIterator first, InputIterator last);
	ShmString& append(const ShmString& str);
	ShmString& append(const ShmString& str, size_t subpos, size_t sublen = npos);
	ShmString& append(const char* s);
	ShmString& append(const char* s, size_t n);
	ShmString& append(size_t n, char c);
	template <class InputIterator>
	ShmString& append(InputIterator first, InputIterator last);
	ShmString& operator+=(const ShmString& str);
	ShmString& operator+=(const char* s);
	ShmString& operator+=(char c);

	void pop_back() { erase(end() - 1, end()); }
	ShmString& erase(size_t pos = 0, size_t len = npos);
	iterator erase(iterator p);
	iterator erase(iterator first, iterator last);

	ShmString& replace(size_t pos, size_t len, const ShmString& str);
	ShmString& replace(iterator i1, iterator i2, const ShmString& str);
	ShmString& replace(size_t pos, size_t len, const ShmString& str, size_t subpos, size_t sublen = npos);
	ShmString& replace(size_t pos, size_t len, const char* s);
	ShmString& replace(iterator i1, iterator i2, const char* s);
	ShmString& replace(size_t pos, size_t len, const char* s, size_t n);
	ShmString& replace(iterator i1, iterator i2, const char* s, size_t n);
	ShmString& replace(size_t pos, size_t len, size_t n, char c);
	ShmString& replace(iterator i1, iterator i2, size_t n, char c);
	template <class InputIterator>
	ShmString& replace(iterator i1, iterator i2, InputIterator first, InputIterator last);

	void swap(ShmString& str) {
		smd::swap(m_start, str.m_start);
		smd::swap(m_finish, str.m_finish);
		smd::swap(m_endOfStorage, str.m_endOfStorage);
	}

	size_t copy(char* s, size_t len, size_t pos = 0) const {
		auto ptr = TinySTL::uninitialized_copy(begin() + pos, begin() + pos + len, s);
		return (size_t)(ptr - s);
	}

	size_t find(const ShmString& str, size_t pos = 0) const;
	size_t find(const char* s, size_t pos = 0) const;
	size_t find(const char* s, size_t pos, size_t n) const;
	size_t find(char c, size_t pos = 0) const;
	size_t rfind(const ShmString& str, size_t pos = npos) const;
	size_t rfind(const char* s, size_t pos = npos) const;
	size_t rfind(const char* s, size_t pos, size_t n) const;
	size_t rfind(char c, size_t pos = npos) const;
	size_t find_first_of(const ShmString& str, size_t pos = 0) const;
	size_t find_first_of(const char* s, size_t pos = 0) const;
	size_t find_first_of(const char* s, size_t pos, size_t n) const;
	size_t find_first_of(char c, size_t pos = 0) const;
	size_t find_last_of(const ShmString& str, size_t pos = npos) const;
	size_t find_last_of(const char* s, size_t pos = npos) const;
	size_t find_last_of(const char* s, size_t pos, size_t n) const;
	size_t find_last_of(char c, size_t pos = npos) const;
	size_t find_first_not_of(const ShmString& str, size_t pos = 0) const;
	size_t find_first_not_of(const char* s, size_t pos = 0) const;
	size_t find_first_not_of(const char* s, size_t pos, size_t n) const;
	size_t find_first_not_of(char c, size_t pos = 0) const;
	size_t find_last_not_of(const ShmString& str, size_t pos = npos) const;
	size_t find_last_not_of(const char* s, size_t pos = npos) const;
	size_t find_last_not_of(const char* s, size_t pos, size_t n) const;
	size_t find_last_not_of(char c, size_t pos = npos) const;

	ShmString substr(size_t pos = 0, size_t len = npos) const {
		len = changeVarWhenEuqalNPOS(len, size(), pos);
		return ShmString(begin() + pos, begin() + pos + len);
	}

	int compare(const ShmString& str) const;
	int compare(size_t pos, size_t len, const ShmString& str) const;
	int compare(
		size_t pos, size_t len, const ShmString& str, size_t subpos, size_t sublen = npos) const;
	int compare(const char* s) const;
	int compare(size_t pos, size_t len, const char* s) const;
	int compare(size_t pos, size_t len, const char* s, size_t n) const;

private:
	void moveData(ShmString& str);
	//插入时空间不足的情况
	template <class InputIterator>
	iterator insert_aux_copy(iterator p, InputIterator first, InputIterator last);
	//插入时空间不足的情况
	iterator insert_aux_filln(iterator p, size_t n, value_type c);
	size_type getNewCapacity(size_type len) const;
	void allocateAndFillN(size_t n, char c);
	template <class InputIterator>
	void allocateAndCopy(InputIterator first, InputIterator last);
	void string_aux(size_t n, char c, std::true_type);
	template <class InputIterator>
	void string_aux(InputIterator first, InputIterator last, std::false_type);
	void destroyAndDeallocate();
	size_t rfind_aux(const_iterator cit, size_t pos, size_t lengthOfS, int cond) const;
	size_t find_aux(const_iterator cit, size_t pos, size_t lengthOfS, size_t cond) const;
	int compare_aux(size_t pos, size_t len, const_iterator cit, size_t subpos, size_t sublen) const;
	bool isContained(char ch, const_iterator first, const_iterator last) const;
	size_t changeVarWhenEuqalNPOS(size_t var, size_t minuend, size_t minue) const;

public:
	friend std::ostream& operator<<(std::ostream& os, const ShmString& str);
	friend std::istream& operator>>(std::istream& is, ShmString& str);
	friend ShmString operator+(const ShmString& lhs, const ShmString& rhs);
	friend ShmString operator+(const ShmString& lhs, const char* rhs);
	friend ShmString operator+(const char* lhs, const ShmString& rhs);
	friend ShmString operator+(const ShmString& lhs, char rhs);
	friend ShmString operator+(char lhs, const ShmString& rhs);
	friend bool operator==(const ShmString& lhs, const ShmString& rhs);
	friend bool operator==(const char* lhs, const ShmString& rhs);
	friend bool operator==(const ShmString& lhs, const char* rhs);
	friend bool operator!=(const ShmString& lhs, const ShmString& rhs);
	friend bool operator!=(const char* lhs, const ShmString& rhs);
	friend bool operator!=(const ShmString& lhs, const char* rhs);
	friend bool operator<(const ShmString& lhs, const ShmString& rhs);
	friend bool operator<(const char* lhs, const ShmString& rhs);
	friend bool operator<(const ShmString& lhs, const char* rhs);
	friend bool operator<=(const ShmString& lhs, const ShmString& rhs);
	friend bool operator<=(const char* lhs, const ShmString& rhs);
	friend bool operator<=(const ShmString& lhs, const char* rhs);
	friend bool operator>(const ShmString& lhs, const ShmString& rhs);
	friend bool operator>(const char* lhs, const ShmString& rhs);
	friend bool operator>(const ShmString& lhs, const char* rhs);
	friend bool operator>=(const ShmString& lhs, const ShmString& rhs);
	friend bool operator>=(const char* lhs, const ShmString& rhs);
	friend bool operator>=(const ShmString& lhs, const char* rhs);
	friend void swap(ShmString& x, ShmString& y);
	friend std::istream& getline(std::istream& is, ShmString& str, char delim);
	friend std::istream& getline(std::istream& is, ShmString& str);

private:
	//Alloc& m_alloc;
	char* m_start;
	char* m_finish;
	char* m_endOfStorage;
};

template <class InputIterator>
ShmString::ShmString(InputIterator first, InputIterator last) {
	//处理指针和数字间的区别的函数
	string_aux(first, last, typename std::is_integral<InputIterator>::type());
}

template <class InputIterator>
ShmString::iterator ShmString::insert_aux_copy(
	iterator p, InputIterator first, InputIterator last) {
	size_t lengthOfInsert = last - first;
	auto newCapacity = getNewCapacity(lengthOfInsert);
	iterator newStart = dataAllocator::allocate(newCapacity);
	iterator newFinish = TinySTL::uninitialized_copy(m_start, p, newStart);
	newFinish = TinySTL::uninitialized_copy(first, last, newFinish);
	auto res = newFinish;
	newFinish = TinySTL::uninitialized_copy(p, m_finish, newFinish);

	destroyAndDeallocate();
	m_start = newStart;
	m_finish = newFinish;
	m_endOfStorage = m_start + newCapacity;
	return res;
}

template <class InputIterator>
ShmString::iterator ShmString::insert(iterator p, InputIterator first, InputIterator last) {
	auto lengthOfLeft = capacity() - size();
	size_t lengthOfInsert = distance(first, last);
	if (lengthOfInsert <= lengthOfLeft) {
		for (iterator it = m_finish - 1; it >= p; --it) {
			*(it + lengthOfInsert) = *(it);
		}
		TinySTL::uninitialized_copy(first, last, p);
		m_finish += lengthOfInsert;
		return (p + lengthOfInsert);
	} else {
		return insert_aux_copy(p, first, last);
	}
}

template <class InputIterator>
ShmString& ShmString::append(InputIterator first, InputIterator last) {
	insert(end(), first, last);
	return *this;
}

template <class InputIterator>
ShmString& ShmString::replace(iterator i1, iterator i2, InputIterator first, InputIterator last) {
	auto ptr = erase(i1, i2);
	insert(ptr, first, last);
	return *this;
}

template <class InputIterator>
void ShmString::allocateAndCopy(InputIterator first, InputIterator last) {
	m_start = dataAllocator::allocate(last - first);
	m_finish = TinySTL::uninitialized_copy(first, last, m_start);
	m_endOfStorage = m_finish;
}

template <class InputIterator>
void ShmString::string_aux(InputIterator first, InputIterator last, std::false_type) {
	allocateAndCopy(first, last);
}

} // namespace smd
