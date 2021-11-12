#pragma once
#include <container/shm_pointer.h>

namespace smd {

template <class T>
class shm_list;

template <class T>
struct ListNode {
	shm_pointer<shm_list<T>> container;
	T data;
	shm_pointer<ListNode> prev;
	shm_pointer<ListNode> next;

	ListNode(shm_pointer<shm_list<T>> c, const T& d, shm_pointer<ListNode> p, shm_pointer<ListNode> n)
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
public:
	typedef shm_pointer<ListNode<T>> nodePtr;

	nodePtr p;

public:
	explicit ListIterator(nodePtr ptr = shm_nullptr)
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

	T& operator*() { return (*p).data; }
	T* operator->() { return &((*p).data); }

	void swap(ListIterator<T>& x) { std:swap(p, x.p); }

	friend bool operator!=(const ListIterator<T>& x, const ListIterator<T>& y) {
		return x.p != y.p;
	}
	friend bool operator==(const ListIterator<T>& x, const ListIterator<T>& y) {
		return x.p == y.p;
	}
};

template <class T>
class shm_list {
public:
	typedef shm_pointer<ListNode<T>> nodePtr;
	typedef ListIterator<T> iterator;

	shm_list()
		: m_head(NewNode(T()))
		, m_tail(m_head) {}

	shm_list(const shm_list<T>& r)
		: m_head(NewNode(*((shm_list<T>&)r).end()))
		, m_tail(m_head) {
		shm_list<T>* r1 = (shm_list<T>*)&r;
		for (iterator it = r1->begin(); it != r1->end(); ++it) {
			auto& element = *it;
			push_back(element);
		}
	}

	shm_list& operator=(const shm_list& l) {
		if (this != &l) {
			shm_list(l).swap(*this);
		}
		return *this;
	}

	~shm_list() {
		clear();
		g_alloc->Delete(m_tail.p);

		m_head = shm_nullptr;
		m_tail = shm_nullptr;
	}

	T& front() { return (m_head.p->data); }
	T& back() { return (m_tail.p->prev->data); }

	void push_front(const T& val) {
		auto node = NewNode(val);
		m_head.p->prev = node;
		node->next = m_head.p;
		m_head.p = node;
	}

	void pop_front() {
		auto node = m_head.p;
		m_head.p = node->next;
		m_head.p->prev = shm_nullptr;
		DeleteNode(node);
	}

	void push_back(const T& val) {
		auto node = NewNode(val);
		if (m_tail.p->prev != shm_nullptr) {
			// 已有元素
			auto prev = m_tail.p->prev;
			prev->next = node;
			node->next = m_tail.p;

			m_tail.p->prev = node;
			node->prev = prev;
		} else {
			// 空链表
			node->next = m_tail.p;
			node->prev = shm_nullptr;

			m_tail.p->prev = node;
			m_head.p = node;
		}
	}

	void pop_back() {
		auto node = m_tail.p->prev;
		if (node->prev != shm_nullptr) {
			auto& prev = node->prev;
			prev->next = node->next;

			m_tail.p->prev = node->prev;
		} else {
			m_head.p = m_tail.p;
			m_head.p->next = shm_nullptr;
			m_head.p->prev = shm_nullptr;
		}

		DeleteNode(node);
	}

	iterator begin() { return m_head; }
	iterator end() { return m_tail; }

	bool empty() const { return m_head == m_tail; }
	size_t size() const {
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
	nodePtr NewNode(const T& val) {
		auto p = g_alloc->New<ListNode<T>>(
			g_alloc->ToShmPointer<shm_list<T>>(this), val, shm_nullptr, shm_nullptr);
		return p;
	}

	void DeleteNode(nodePtr p) {
		p->prev = p->next = shm_nullptr;
		g_alloc->Delete(p);
	}

	void swap(shm_list<T>& x) {
		std::swap(m_head, x.m_head);
		std::swap(m_tail, x.m_tail);
	}

private:
	iterator m_head;
	iterator m_tail;
};

} // namespace smd
