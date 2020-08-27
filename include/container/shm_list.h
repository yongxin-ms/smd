#pragma once
#include "shm_obj.h"

namespace smd {

template <class T>
class ShmList;

template <class T>
struct ListNode {
	ShmList<T>& container;
	T data;
	ShmPointer<ListNode> prev;
	ShmPointer<ListNode> next;

	ListNode(ShmList<T>& c, const T& d, ShmPointer<ListNode> p, ShmPointer<ListNode> n)
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
class ListIterator : public ShmObj{
public:
	typedef ShmPointer<ListNode<T>> nodePtr;

	nodePtr p;

public:
	explicit ListIterator(Alloc& alloc, nodePtr ptr = shm_nullptr)
		: ShmObj(alloc)
		, p(ptr) {}

	ListIterator& operator=(nodePtr ptr) {
		p = ptr;
		return *this;
	}

	ListIterator& operator++() {
		p = p.ObjPtr(m_alloc)->next;
		return *this;
	}

	ListIterator operator++(int) {
		auto res = *this;
		++*this;
		return res;
	}

	ListIterator& operator--() {
		p = p.ObjPtr(m_alloc)->prev;
		return *this;
	}

	ListIterator operator--(int) {
		auto res = *this;
		--*this;
		return res;
	}

	T& operator*() { return p.ObjRef(m_alloc).data; }
	T* operator->() { return &(p.ObjRef(m_alloc).data); }

	void swap(ListIterator<T>& x) { smd::swap(p, x.p); }

	friend bool operator!=(const ListIterator<T>& x, const ListIterator<T>& y) {
		return x.p != y.p;
	}
	friend bool operator==(const ListIterator<T>& x, const ListIterator<T>& y) {
		return x.p == y.p;
	}
};

template <class T>
class ShmList : public ShmObj {
public:
	typedef ShmPointer<ListNode<T>> nodePtr;
	typedef ListIterator<T> iterator;

	ShmList(Alloc& alloc, const T& dummy)
		: ShmObj(alloc)
		, m_head(alloc, NewNode(dummy))
		, m_tail(m_head) {}

	ShmList(const ShmList<T>& r)
		: ShmObj(r.m_alloc)
		, m_head(r.m_alloc, NewNode(*((ShmList<T>&)r).end()))
		, m_tail(m_head) {
		ShmList<T>* r1 = (ShmList<T>*)&r;
		for (iterator it = r1->begin(); it != r1->end(); ++it) {
			auto& element = *it;
			push_back(element);
		}
	}

	ShmList& operator=(const ShmList& l) {
		if (this != &l) {
			ShmList(l).swap(*this);
		}
		return *this;
	}

	~ShmList() {
		clear();
		m_alloc.Delete(m_tail.p);

		m_head = shm_nullptr;
		m_tail = shm_nullptr;
	}

	T& front() { return (m_head.p.ObjPtr(m_alloc)->data); }
	T& back() { return (m_tail.p.ObjPtr(m_alloc)->prev.ObjPtr(m_alloc)->data); }

	void push_front(const T& val) {
		auto node = NewNode(val);
		m_head.p.ObjPtr(m_alloc)->prev = node;
		node.ObjPtr(m_alloc)->next = m_head.p;
		m_head.p = node;
	}

	void pop_front() {
		auto node = m_head.p;
		m_head.p = node.ObjPtr(m_alloc)->next;
		m_head.p.ObjPtr(m_alloc)->prev = shm_nullptr;
		DeleteNode(node);
	}

	void push_back(const T& val) {
		auto node = NewNode(val);
		if (m_tail.p.ObjPtr(m_alloc)->prev != shm_nullptr) {
			// 已有元素
			auto prev = m_tail.p.ObjPtr(m_alloc)->prev;
			prev.ObjPtr(m_alloc)->next = node;
			node.ObjPtr(m_alloc)->next = m_tail.p;

			m_tail.p.ObjPtr(m_alloc)->prev = node;
			node.ObjPtr(m_alloc)->prev = prev;
		} else {
			// 空链表
			node.ObjPtr(m_alloc)->next = m_tail.p;
			node.ObjPtr(m_alloc)->prev = shm_nullptr;

			m_tail.p.ObjPtr(m_alloc)->prev = node;
			m_head.p = node;
		}
	}

	void pop_back() {
		auto node = m_tail.p.ObjPtr(m_alloc)->prev;
		if (node.ObjPtr(m_alloc)->prev != shm_nullptr) {
			auto& prev = node.ObjPtr(m_alloc)->prev;
			prev.ObjPtr(m_alloc)->next = node.ObjPtr(m_alloc)->next;

			m_tail.p.ObjPtr(m_alloc)->prev = node.ObjPtr(m_alloc)->prev;
		} else {
			m_head.p = m_tail.p;
			m_head.p.ObjPtr(m_alloc)->next = shm_nullptr;
			m_head.p.ObjPtr(m_alloc)->prev = shm_nullptr;
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
			auto prev = position.p.ObjPtr(m_alloc)->prev;
			prev.ObjPtr(m_alloc)->next = position.p.ObjPtr(m_alloc)->next;
			position.p.ObjPtr(m_alloc)->next.ObjPtr(m_alloc)->prev = prev;
			DeleteNode(position.p);
			return iterator(m_alloc, prev.ObjPtr(m_alloc)->next);
		}
	}

	iterator erase(iterator first, iterator last) {
		iterator res(m_alloc);
		for (; first != last;) {
			auto temp = first++;
			res = erase(temp);
		}
		return res;
	}

private:
	nodePtr NewNode(const T& val) {
		auto p = m_alloc.New<ListNode<T>>(*this, val, shm_nullptr, shm_nullptr);
		return p;
	}

	void DeleteNode(nodePtr p) {
		p.ObjPtr(m_alloc)->prev = p.ObjPtr(m_alloc)->next = shm_nullptr;
		m_alloc.Delete(p);
	}

	void swap(ShmList<T>& x) {
		smd::swap(m_head, x.m_head);
		smd::swap(m_tail, x.m_tail);
	}

private:
	iterator m_head;
	iterator m_tail;
};

} // namespace smd
