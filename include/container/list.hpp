#pragma once
#include <string>
#include "../common/smd_defines.h"
#include "string.hpp"

namespace smd {

template <class _Alloc>
class ListNode : public BaseObj {
public:
	friend class List;
	ListNode(const std::string& value)
		: m_value(value)
		, m_next(nullptr)
		, m_prev(nullptr) {}

	String<_Alloc>& value() { return m_value; }

	ListNode<_Alloc>* next() { return m_next; }
	ListNode<_Alloc>* prev() { return m_prev; }

	bool is_tail() const { return m_next == nullptr; }
	bool is_head() const { return m_prev == nullptr; }

	virtual void serialize(std::string& to) override { m_value.serialize(to); }

	virtual void deserialize(const std::string& from, size_t& pos) override {}

private:
	void set_next(ListNode<_Alloc>* node) { m_next = node; }
	void set_prev(ListNode<_Alloc>* node) { m_prev = node; }

private:
	String<_Alloc> m_value;
	ListNode<_Alloc>* m_next;
	ListNode<_Alloc>* m_prev;
};

template <class _Alloc>
class List {
public:
	List(const std::string& name)
		: m_name(name)
		, m_nodeNum(0)
		, m_head(nullptr)
		, m_tail(nullptr) {}
	~List() { clear(); }

	// 往尾部添加元素
	ListNode<_Alloc>* append(const std::string& value) {
		auto node = new ListNode<_Alloc>(value);
		if (m_tail == nullptr) {
			// 这是一个空链表
			assert(m_head == nullptr);

			m_head = node;
			m_tail = node;
		} else {
			m_tail->set_next(node);
			node->set_prev(m_tail);
			m_tail = node;
		}

		return node;
	}

	ListNode<_Alloc>* begin() { return m_head; }
	ListNode<_Alloc>* tail() { return m_tail; }

	// 删除一个元素（返回下一个元素）
	ListNode<_Alloc>* erase(ListNode<_Alloc>* node) {
		if (node->prev() != nullptr) {
			node->prev()->set_next(node->next());

			if (m_tail == node) {
				m_tail = node->prev();
			}
		}

		if (node->next() != nullptr) {
			node->next()->set_prev(node->prev());

			if (m_head == node) {
				m_head = node->next();
			}
		}

		auto next = node->next();
		delete node;
		m_nodeNum--;
		return next;
	}

	bool empty() { return m_nodeNum == 0; }
	size_t size() { return m_nodeNum; }
	void clear() {
		while (m_head != nullptr) {
			m_head = erase(m_head);
		}
	}

private:
	String<_Alloc> m_name;
	size_t m_nodeNum;
	ListNode<_Alloc>* m_head;
	ListNode<_Alloc>* m_tail;
};
} // namespace smd
