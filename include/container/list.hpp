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
		: BaseObj(BaseObj::ObjType::OBJ_LIST_NODE)
		, m_value(new String<_Alloc>(value))
		, m_next(nullptr)
		, m_prev(nullptr) {
		// 只需要存储三个指针
		SetSize(3 * sizeof(SMD_POINTER));
		auto ptr = _Alloc::Acquire(GetSize());
		SetPtr(ptr);
	}

	String<_Alloc>& value() { return *m_value; }

	ListNode<_Alloc>* next() { return m_next; }
	ListNode<_Alloc>* prev() { return m_prev; }

	bool is_tail() const { return m_next == nullptr; }
	bool is_head() const { return m_prev == nullptr; }

	void Serialize(std::string& to) {
		BaseObj::Serialize(to);

		SMD_POINTER ptr = m_value.GetPtr();
		to.append((const char*)&ptr, sizeof(ptr));

		ptr = m_next != nullptr ? m_next->GetPtr() : 0;
		to.append((const char*)&ptr, sizeof(ptr));

		ptr = m_prev != nullptr ? m_prev->GetPtr() : 0;
		to.append((const char*)&ptr, sizeof(ptr));
	}

	void Deserialize(const char*& buf, size_t& len) {
		BaseObj::Deserialize(buf, len);

		SMD_POINTER ptr = 0;
		ReadStream(ptr, buf, len);
		// 添加代码给m_value赋值

		ReadStream(ptr, buf, len);
		// 添加代码给m_next赋值

		ReadStream(ptr, buf, len);
		// 添加代码给m_prev赋值
	}

private:
	void set_next(ListNode<_Alloc>* node) { m_next = node; }
	void set_prev(ListNode<_Alloc>* node) { m_prev = node; }

private:
	_Alloc& m_alloc;

	String<_Alloc>* m_value;
	ListNode<_Alloc>* m_next;
	ListNode<_Alloc>* m_prev;
};

template <class _Alloc>
class List : public BaseObj {
public:
	List(const std::string& name)
		: BaseObj(BaseObj::ObjType::OBJ_LIST)
		, m_name(new String<_Alloc>(name))
		, m_head(nullptr)
		, m_tail(nullptr)
		, m_nodeNum(0) {}
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
	String<_Alloc>* m_name;
	ListNode<_Alloc>* m_head;
	ListNode<_Alloc>* m_tail;
	size_t m_nodeNum;
};
} // namespace smd
