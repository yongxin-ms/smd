#pragma once
#include "shm_obj.h"

namespace smd {

template <class T>
class ShmList;

template <class T>
struct ListNode {
	ShmList<T>& container;
	T			data;
	ListNode*	prev;
	ListNode*	next;

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
class ListIterator {
	template <class T>
	friend class ShmList;

public:
	typedef ListNode<T>* nodePtr;
	nodePtr				 p;

public:
	explicit ListIterator(nodePtr ptr = nullptr)
		: p(ptr) {}

	ListIterator& operator=(nodePtr ptr) {
		p = ptr;
		return *this;
	}

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
	typedef ListNode<T>*	nodePtr;
	typedef ListIterator<T> iterator;

	ShmList(Alloc& alloc)
		: ShmObj(alloc) {
		m_head = NewNode(T(alloc)); // add a dummy node
		m_tail = m_head;
	}

	~ShmList() {
		clear();
		m_alloc.Delete(m_tail.p);

		m_head = nullptr;
		m_tail = nullptr;
	}

	T& front() { return (m_head.p->data); }
	T& back() { return (m_tail.p->prev->data); }

	void push_front(const T& val) {
		auto node	   = NewNode(val);
		m_head.p->prev = node;
		node->next	   = m_head.p;
		m_head.p	   = node;
	}

	void pop_front() {
		auto node	   = m_head.p;
		m_head.p	   = node->next;
		m_head.p->prev = nullptr;
		DeleteNode(node);
	}

	void push_back(const T& val) {
		auto node = NewNode(val);
		if (m_tail.p->prev != nullptr) {
			// 已有元素
			auto prev = m_tail.p->prev;
			prev->next = node;
			node->next = m_tail.p;

			m_tail.p->prev = node;
			node->prev	   = prev;
		} else {
			// 空链表
			node->next = m_tail.p;
			node->prev = nullptr;

			m_tail.p->prev = node;
			m_head.p	   = node;
		}
	}

	void pop_back() {
		auto node = m_tail.p->prev;
		if (node->prev != nullptr) {
			auto& prev = node->prev;
			prev->next = node->next;

			m_tail.p->prev = node->prev;
		} else {
			m_head.p	   = m_tail.p;
			m_head.p->next = nullptr;
			m_head.p->prev = nullptr;
		}

		DeleteNode(node);
	}

	iterator begin() { return m_head; }
	iterator end() { return m_tail; }

	bool   empty() { return m_head != m_tail; }
	size_t size() {
		size_t length = 0;
		for (auto h = m_head; h != m_tail; ++h)
			++length;
		return length;
	}

	void	 clear() { erase(begin(), end()); }
	iterator erase(iterator position) {
		if (position == m_head) {
			pop_front();
			return m_head;
		} else {
			auto prev			   = position.p->prev;
			prev->next			   = position.p->next;
			position.p->next->prev = prev;
			DeleteNode(position.p);
			return iterator(prev->next);
		}
	}

	iterator erase(iterator first, iterator last) {
		iterator res;
		for (; first != last;) {
			auto temp = first++;
			res		  = erase(temp);
		}
		return res;
	}

private:
	nodePtr NewNode(const T& val) {
		auto p = m_alloc.New<ListNode<T>>(*this, val, nullptr, nullptr);
		return p;
	}

	void DeleteNode(nodePtr p) {
		p->prev = p->next = nullptr;
		m_alloc.Delete(p);
	}

private:
	iterator m_head;
	iterator m_tail;
};

} // namespace smd
