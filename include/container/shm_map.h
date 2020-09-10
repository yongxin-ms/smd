#pragma once
#include "shm_pointer.h"

namespace smd {

enum RBTreeNodeColor : bool {
	RBTREE_NODE_RED = false,
	RBTREE_NODE_BLACK = true,
};

template <typename Value>
struct RBTreeNode {
	RBTreeNodeColor color;
	ShmPointer<RBTreeNode> parent;
	ShmPointer<RBTreeNode> left_child;
	ShmPointer<RBTreeNode> right_child;
	Value value;

	RBTreeNode(const Value& val)
		: color(RBTREE_NODE_RED)
		, value(val) {}
	void swap(RBTreeNode<Value>& x) {
		std::swap(color, x.color);
		std::swap(parent, x.parent);
		std::swap(left_child, x.left_child);
		std::swap(right_child, x.right_child);
		std::swap(value, x.value);
	}
};

template <typename value_type>
static ShmPointer<RBTreeNode<value_type>> rbtree_prev(ShmPointer<RBTreeNode<value_type>> node) {
	if (node == shm_nullptr) {
		return shm_nullptr;
	}

	if (node->left_child != shm_nullptr) {
		node = node->left_child;

		while (node->right_child != shm_nullptr) {
			node = node->right_child;
		}

		return node;
	}

	ShmPointer<RBTreeNode<value_type>> n;
	while ((n = node->parent) != shm_nullptr && node == n->left_child) {
		node = n;
	}

	return n;
}

template <typename value_type>
static ShmPointer<RBTreeNode<value_type>> rbtree_next(ShmPointer<RBTreeNode<value_type>> node) {
	if (node == shm_nullptr) {
		return shm_nullptr;
	}

	if (node->right_child != shm_nullptr) {
		node = node->right_child;

		while (node->left_child != shm_nullptr) {
			node = node->left_child;
		}

		return node;
	}

	ShmPointer<RBTreeNode<value_type>> n;
	while ((n = node->parent) != shm_nullptr && node == n->right_child) {
		node = n;
	}

	return n;
}

template <typename T, typename Pointer, typename Reference>
struct rbtree_iterator {
	typedef rbtree_iterator<T, Pointer, Reference> this_type;

	ShmPointer<RBTreeNode<T>> _ptr;

	rbtree_iterator()
		: _ptr(shm_nullptr) {}
	rbtree_iterator(ShmPointer<RBTreeNode<T>> pNode)
		: _ptr(pNode) {}
	rbtree_iterator(const this_type& x) = default;

	Reference operator*() const { return _ptr->value; }
	Pointer operator->() const { return &(_ptr->value); }

	rbtree_iterator& operator++() {
		_ptr = rbtree_next<T>(_ptr);
		return *this;
	}

	rbtree_iterator operator++(int) {
		this_type tmp(*this);
		_ptr = rbtree_next<T>(_ptr);
		return tmp;
	}

	rbtree_iterator& operator--() {
		_ptr = rbtree_prev<T>(_ptr);
		return *this;
	}

	rbtree_iterator operator--(int) {
		this_type tmp(*this);
		_ptr = rbtree_prev<T>(_ptr);
		return tmp;
	}

	bool operator==(const this_type& x) { return _ptr == x._ptr; }
	bool operator!=(const this_type& x) { return _ptr != x._ptr; }
};

template <typename Key, typename Value>
class RBTree {
public:
	typedef RBTree<Key, Value> this_type;
	typedef std::pair<Key, Value> value_type;
	typedef ShmPointer<RBTreeNode<value_type>> rbtree_node_ptr;
	typedef rbtree_iterator<value_type, value_type*, value_type&> iterator;
	typedef rbtree_iterator<value_type, const value_type*, const value_type&> const_iterator;

	RBTree()
		: root(shm_nullptr)
		, size(0) {}
	RBTree(const this_type& r) = delete;
	this_type& operator=(const this_type& r) = delete;
	~RBTree() { rbtree_remove_all(); }

	size_t rbtree_size() const { return size; }
	bool rbtree_empty() { return size == 0; }

	rbtree_node_ptr rbtree_insert(const value_type& value) {
		auto node = createNode(value);

		auto n = root;
		if (n != shm_nullptr) {
			for (;;) {
				auto cmp = compare(key(node), n);

				if (cmp < 0) {
					if (n->left_child != shm_nullptr) {
						n = n->left_child;
					} else {
						n->left_child = node;

						break;
					}
				} else if (cmp > 0) {
					if (n->right_child != shm_nullptr) {
						n = n->right_child;
					} else {
						n->right_child = node;

						break;
					}
				} else {
					//节点重复，插入失败
					deleteNode(node);
					return shm_nullptr;
				}
			}
		}

		node->parent = n;
		node->left_child = shm_nullptr;
		node->right_child = shm_nullptr;
		node->color = RBTREE_NODE_RED;

		if (n == shm_nullptr) {
			root = node;
		}

		repair_after_insert(node);

		++size;
		return node;
	}

	rbtree_node_ptr rbtree_lookup_key(const Key& key) {
		auto n = root;
		while (n != shm_nullptr) {
			auto cmp = compare(key, n);

			if (cmp < 0) {
				n = n->left_child;
			} else if (cmp > 0) {
				n = n->right_child;
			} else {
				break;
			}
		}

		return n;
	}

	// 删除一个节点之后，返回下一个节点
	rbtree_node_ptr rbtree_remove(rbtree_node_ptr& node) {
		if (node == shm_nullptr) {
			return shm_nullptr;
		}

		auto next_node = rbtree_next(node);

		if (node->left_child != shm_nullptr && node->right_child != shm_nullptr) {
			auto k = node->left_child;

			while (k->right_child != shm_nullptr) {
				k = k->right_child;
			}

			swap_places(node, k);
		}

		auto n = node->right_child != shm_nullptr ? node->right_child : node->left_child;

		if (color(node) == RBTREE_NODE_BLACK) {
			node->color = color(n);

			repair_after_remove(node);
		}

		transplant(node, n);

		if (node->parent == shm_nullptr && n != shm_nullptr) {
			n->color = RBTREE_NODE_BLACK;
		}

		deleteNode(node);
		--size;
		return next_node;
	}

	rbtree_node_ptr rbtree_first() {
		auto n = root;
		if (n == shm_nullptr) {
			return shm_nullptr;
		}

		while (n->left_child != shm_nullptr) {
			n = n->left_child;
		}

		return n;
	}

	rbtree_node_ptr rbtree_last() {
		auto n = root;
		if (n == shm_nullptr) {
			return shm_nullptr;
		}

		while (n->right_child != shm_nullptr) {
			n = n->right_child;
		}

		return n;
	}

	void rbtree_remove_key(const Key& key) { rbtree_remove(rbtree_lookup_key(key)); }
	void rbtree_remove_first() { rbtree_remove(rbtree_first()); }
	void rbtree_remove_last() { rbtree_remove(rbtree_last()); }
	void rbtree_remove_all() {
		if (root != shm_nullptr) {
			root->parent = shm_nullptr;
			root->left_child = shm_nullptr;
			root->right_child = shm_nullptr;
		}

		root = shm_nullptr;
		size = 0;
	}

protected:
	rbtree_node_ptr root;
	size_t size;
	static int64_t compare(const Key& k, rbtree_node_ptr node) {
		return smd::compare(k, key(node));
	}

protected:
	static RBTreeNodeColor color(rbtree_node_ptr node) {
		return node != shm_nullptr ? node->color : RBTREE_NODE_BLACK;
	}

	static rbtree_node_ptr sibling(rbtree_node_ptr node) {
		assert(node != shm_nullptr && node->parent != shm_nullptr);

		if (node == node->parent->left_child) {
			return node->parent->right_child;
		} else {
			return node->parent->left_child;
		}
	}

	static rbtree_node_ptr grandparent(rbtree_node_ptr node) {
		assert(node != shm_nullptr && node->parent != shm_nullptr &&
			   node->parent->parent != shm_nullptr);

		return node->parent->parent;
	}

	static rbtree_node_ptr uncle(rbtree_node_ptr node) {
		assert(node != shm_nullptr && node->parent != shm_nullptr &&
			   node->parent->parent != shm_nullptr);

		return sibling(node->parent);
	}

	rbtree_node_ptr createNode(const value_type& val) {
		return g_alloc->New<RBTreeNode<value_type>>(val);
	}
	void deleteNode(rbtree_node_ptr& p) { g_alloc->Delete(p); }

	static value_type& value(rbtree_node_ptr x) { return x->value; }
	static const Key& key(rbtree_node_ptr x) { return value(x).first; }

	void transplant(rbtree_node_ptr old_node, rbtree_node_ptr new_node) {
		assert(old_node != shm_nullptr);

		if (old_node->parent == shm_nullptr) {
			root = new_node;
		} else if (old_node == old_node->parent->left_child) {
			old_node->parent->left_child = new_node;
		} else {
			old_node->parent->right_child = new_node;
		}

		if (new_node != shm_nullptr) {
			new_node->parent = old_node->parent;
		}
	}

	void swap_places(rbtree_node_ptr high_node, rbtree_node_ptr low_node) {
		assert(high_node != shm_nullptr && low_node != shm_nullptr);

		if (high_node->parent == shm_nullptr) {
			root = low_node;
		} else if (high_node->parent->left_child == high_node) {
			high_node->parent->left_child = low_node;
		} else {
			high_node->parent->right_child = low_node;
		}

		if (low_node->left_child != shm_nullptr) {
			low_node->left_child->parent = high_node;
		}

		if (low_node->right_child != shm_nullptr) {
			low_node->right_child->parent = high_node;
		}

		if (high_node->left_child == low_node) {
			if (high_node->right_child != shm_nullptr) {
				high_node->right_child->parent = low_node;
			}

			high_node->left_child = high_node;
			low_node->parent = low_node;
		} else if (high_node->right_child == low_node) {
			if (high_node->left_child != shm_nullptr) {
				high_node->left_child->parent = low_node;
			}

			high_node->right_child = high_node;
			low_node->parent = low_node;
		} else {
			if (high_node->left_child != shm_nullptr) {
				high_node->left_child->parent = low_node;
			}

			if (high_node->right_child != shm_nullptr) {
				high_node->right_child->parent = low_node;
			}

			if (low_node->parent->left_child == low_node) {
				low_node->parent->left_child = high_node;
			} else {
				low_node->parent->right_child = high_node;
			}
		}

		std::swap(*high_node, *low_node);
	}

	void rotate_left(rbtree_node_ptr node) {
		assert(node != shm_nullptr);

		auto n = node->right_child;

		transplant(node, n);

		node->right_child = n->left_child;

		if (n->left_child != shm_nullptr) {
			n->left_child->parent = node;
		}

		n->left_child = node;
		node->parent = n;
	}

	void rotate_right(rbtree_node_ptr node) {
		assert(node != shm_nullptr);

		auto n = node->left_child;

		transplant(node, n);

		node->left_child = n->right_child;

		if (n->right_child != shm_nullptr) {
			n->right_child->parent = node;
		}

		n->right_child = node;
		node->parent = n;
	}

	void repair_after_insert(rbtree_node_ptr node) {
		assert(node != shm_nullptr);

		for (;;) {
			if (node->parent == shm_nullptr) {
				node->color = RBTREE_NODE_BLACK;

				break;
			}

			if (color(node->parent) == RBTREE_NODE_BLACK) {
				break;
			}

			if (color(uncle(node)) == RBTREE_NODE_RED) {
				node->parent->color = RBTREE_NODE_BLACK;
				uncle(node)->color = RBTREE_NODE_BLACK;
				grandparent(node)->color = RBTREE_NODE_RED;
				node = grandparent(node);

				continue;
			}

			if (node == node->parent->right_child &&
				node->parent == grandparent(node)->left_child) {
				rotate_left(node->parent);

				node = node->left_child;
			} else if (node == node->parent->left_child &&
					   node->parent == grandparent(node)->right_child) {
				rotate_right(node->parent);

				node = node->right_child;
			}

			node->parent->color = RBTREE_NODE_BLACK;
			grandparent(node)->color = RBTREE_NODE_RED;

			if (node == node->parent->left_child && node->parent == grandparent(node)->left_child) {
				rotate_right(grandparent(node));
			} else {
				rotate_left(grandparent(node));
			}

			break;
		}
	}

	void repair_after_remove(rbtree_node_ptr node) {
		assert(node != shm_nullptr);

		for (;;) {
			if (node->parent == shm_nullptr) {
				break;
			}

			if (color(sibling(node)) == RBTREE_NODE_RED) {
				node->parent->color = RBTREE_NODE_RED;
				sibling(node)->color = RBTREE_NODE_BLACK;

				if (node == node->parent->left_child) {
					rotate_left(node->parent);
				} else {
					rotate_right(node->parent);
				}
			}

			if (color(node->parent) == RBTREE_NODE_BLACK &&
				color(sibling(node)) == RBTREE_NODE_BLACK &&
				color(sibling(node)->left_child) == RBTREE_NODE_BLACK &&
				color(sibling(node)->right_child) == RBTREE_NODE_BLACK) {
				sibling(node)->color = RBTREE_NODE_RED;
				node = node->parent;

				continue;
			}

			if (color(node->parent) == RBTREE_NODE_RED &&
				color(sibling(node)) == RBTREE_NODE_BLACK &&
				color(sibling(node)->left_child) == RBTREE_NODE_BLACK &&
				color(sibling(node)->right_child) == RBTREE_NODE_BLACK) {
				sibling(node)->color = RBTREE_NODE_RED;
				node->parent->color = RBTREE_NODE_BLACK;

				break;
			}

			if (node == node->parent->left_child && color(sibling(node)) == RBTREE_NODE_BLACK &&
				color(sibling(node)->left_child) == RBTREE_NODE_RED &&
				color(sibling(node)->right_child) == RBTREE_NODE_BLACK) {
				sibling(node)->color = RBTREE_NODE_RED;
				sibling(node)->left_child->color = RBTREE_NODE_BLACK;

				rotate_right(sibling(node));
			} else if (node == node->parent->right_child &&
					   color(sibling(node)) == RBTREE_NODE_BLACK &&
					   color(sibling(node)->left_child) == RBTREE_NODE_BLACK &&
					   color(sibling(node)->right_child) == RBTREE_NODE_RED) {
				sibling(node)->color = RBTREE_NODE_RED;
				sibling(node)->right_child->color = RBTREE_NODE_BLACK;

				rotate_left(sibling(node));
			}

			sibling(node)->color = color(node->parent);
			node->parent->color = RBTREE_NODE_BLACK;

			if (node == node->parent->left_child) {
				sibling(node)->right_child->color = RBTREE_NODE_BLACK;

				rotate_left(node->parent);
			} else {
				sibling(node)->left_child->color = RBTREE_NODE_BLACK;

				rotate_right(node->parent);
			}

			break;
		}
	}
};

template <typename Key, typename Value>
class ShmMap {
public:
	typedef typename RBTree<Key, Value>::value_type valueType;
	typedef typename RBTree<Key, Value>::iterator iterator;

	ShmMap() {}
	ShmMap(const ShmMap<Key, Value>& r) {}

	ShmMap& operator=(const ShmMap<Key, Value>& r) {
		if (this != &r) {
			ShmMap(r).swap(*this);
		}
		return *this;
	}

	iterator insert(const valueType& v) { return iterator(m_tree.rbtree_insert(v)); }
	bool empty() const { return m_tree.rbtree_size() == 0; }
	size_t size() const { return m_tree.rbtree_size(); }
	void clear() { m_tree.rbtree_remove_all(); }
	iterator begin() { return iterator(m_tree.rbtree_first()); }
	iterator end() { return iterator(shm_nullptr); }
	iterator find(const Key& k) { return iterator(m_tree.rbtree_lookup_key(k)); }
	iterator erase(iterator it) {
		auto it_remove = it++;
		m_tree.rbtree_remove(it_remove._ptr);
		return it;
	}

private:
	RBTree<Key, Value> m_tree;
};

} // namespace smd
