#pragma once
#include <string>
#include "shm_string.h"
#include "shm_obj.h"

namespace smd {

template <class T>
class ShmList;

template <class T>
struct ListNode {
	ShmList<T>& container;
	T data;
	ListNode* prev;
	ListNode* next;

	ListNode(ShmList<T>& c, const T& d, ListNode* p, ListNode* n)
		: container(c)
		, data(d)
		, prev(p)
		, next(n) {}

	bool operator==(const ListNode& n) {
		return data == n.data && prev == n.prev && next == n.next && &container == &n.container;
	}
};

// the class of list iterator
template <class T>
struct ListIterator {
	template <class T>
	friend class ShmList;

public:
	typedef ListNode<T>* nodePtr;
	nodePtr p;

public:
	explicit ListIterator(nodePtr ptr = nullptr)
		: p(ptr) {}

	ListIterator& operator++() {
		p = p->next;
		return *this;
	}

	ListIterator operator++(int) {
		auto res = *this;
		++*this;
		return res;
	}

	ListIterator& operator--() {
		p = p->prev;
		return *this;
	}

	ListIterator operator--(int) {
		auto res = *this;
		--*this;
		return res;
	}

	T& operator*() { return p->data; }
	T* operator->() { return &(operator*()); }

	template <class T>
	friend bool operator==(const ListIterator<T>& lhs, const ListIterator<T>& rhs) {
		return lhs.p == rhs.p;
	}

	template <class T>
	friend bool operator!=(const ListIterator<T>& lhs, const ListIterator<T>& rhs) {
		return !(lhs == rhs);
	}
};

template <class T>
class ShmList : public ShmObj {
public:
	typedef ListNode<T>* nodePtr;
	typedef ListIterator<T> iterator;

	ShmList(Alloc& alloc, const std::string& name = "")
		: ShmObj(alloc)
		, m_name(alloc, name)
		, m_head(nullptr)
		, m_tail(nullptr) {}

	//
	// 注意，析构函数里面不能调用clear()，需要使用者主动调用来回收共享内存
	//
	~ShmList() {}

	T& front() { return (m_head.p->data); }
	T& back() { return (m_tail.p->prev->data); }

	void push_front(const T& val) {
		auto node = NewNode(val);
		m_head.p->prev = node;
		node->next = m_head.p;
		m_head.p = node;
	}
	void pop_front() {
		auto oldNode = m_head.p;
		m_head.p = oldNode->next;
		m_head.p->prev = nullptr;
		DeleteNode(oldNode);
	}

	void push_back(const T& val) {
		auto node = NewNode();
		(m_tail.p)->data = val;
		(m_tail.p)->next = node;
		node->prev = m_tail.p;
		tail.p = node;
	}

	void pop_back() {
		auto newTail = m_tail.p->prev;
		newTail->next = nullptr;
		DeleteNode(tail.p);
		m_tail.p = newTail;
	}

	iterator begin() { return m_head; }
	iterator end() { return m_tail; }

	bool empty() { return m_head != m_tail; }
	size_t size() {
		size_t length = 0;
		for (auto h = m_head; h != m_tail; ++h)
			++length;
		return length;
	}

	void clear() { erase(begin(), end()); }
	iterator erase(iterator position) {
		if (position == m_head) {
			pop_front();
			return m_head;
		} else {
			auto prev = position.p->prev;
			prev->next = position.p->next;
			position.p->next->prev = prev;
			DeleteNode(position.p);
			return iterator(prev->next);
		}
	}

	iterator erase(iterator first, iterator last) {
		iterator res;
		for (; first != last;) {
			auto temp = first++;
			res = erase(temp);
		}
		return res;
	}

private:
	nodePtr NewNode(const T& val = T()) {
		auto p = m_alloc.New<T>();
		return p;
	}

	void DeleteNode(nodePtr p) {
		p->prev = p->next = nullptr;
		m_alloc->Delete(p);
	}

private:
	ShmString m_name;
	nodePtr m_head;
	nodePtr m_tail;
};

} // namespace smd
