#pragma once
#include <container/shm_pointer.h>

namespace smd {

enum RBTreeNodeColor : bool {
	RBTREE_NODE_RED = false,
	RBTREE_NODE_BLACK = true,
};

template <typename Value>
struct RBTreeNode {
	RBTreeNodeColor color;
	shm_pointer<RBTreeNode> parent;
	shm_pointer<RBTreeNode> left_child;
	shm_pointer<RBTreeNode> right_child;
	Value value;

	RBTreeNode(const Value& val)
		: color(RBTREE_NODE_RED)
		, value(val) {}
};

template <typename value_type>
static shm_pointer<RBTreeNode<value_type>> rbtree_prev(shm_pointer<RBTreeNode<value_type>> node) {
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

	shm_pointer<RBTreeNode<value_type>> n;
	while ((n = node->parent) != shm_nullptr && node == n->left_child) {
		node = n;
	}

	return n;
}

template <typename value_type>
static shm_pointer<RBTreeNode<value_type>> rbtree_next(shm_pointer<RBTreeNode<value_type>> node) {
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

	shm_pointer<RBTreeNode<value_type>> n;
	while ((n = node->parent) != shm_nullptr && node == n->right_child) {
		node = n;
	}

	return n;
}

template <typename T, typename Pointer, typename Reference>
struct rbtree_iterator {
	typedef rbtree_iterator<T, Pointer, Reference> this_type;

	shm_pointer<RBTreeNode<T>> _ptr;

	rbtree_iterator()
		: _ptr(shm_nullptr) {}
	rbtree_iterator(shm_pointer<RBTreeNode<T>> pNode)
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
class shm_map {
public:
	typedef shm_map<Key, Value> this_type;
	typedef std::pair<Key, Value> value_type;
	typedef shm_pointer<RBTreeNode<value_type>> rbtree_node_ptr;
	typedef rbtree_iterator<value_type, value_type*, value_type&> iterator;
	typedef rbtree_iterator<value_type, const value_type*, const value_type&> const_iterator;

	shm_map()
		: root_(shm_nullptr)
		, size_(0) {}
	shm_map(const this_type& r)
		: root_(shm_nullptr)
		, size_(0) {
		for (auto it = r.begin(); it != r.end(); ++it) {
			insert(std::make_pair(it->first, it->second));
		}
	}

	this_type& operator=(const this_type& r) {
		if (this != &r) {
			shm_map(r).swap(*this);
		}
		return *this;
	}

	~shm_map() { clear(); }

	iterator begin() { return iterator(rbtree_first()); }
	const_iterator begin() const { return const_iterator(rbtree_first()); }
	iterator end() { return iterator(shm_nullptr); }
	const_iterator end() const { return const_iterator(shm_nullptr); }

	size_t size() const { return size_; }
	bool empty() { return size_ == 0; }
	rbtree_node_ptr insert(const value_type& value) {
		auto node = createNode(value);

		auto n = root_;
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
			root_ = node;
		}

		repair_after_insert(node);

		++size_;
		return node;
	}

	iterator find(const Key& key) { return iterator(rbtree_lookup_key(key)); }
	const_iterator find(const Key& key) const { return const_iterator(rbtree_lookup_key(key)); }
	iterator erase(iterator it) { return iterator(rbtree_remove(it._ptr)); }
	void clear() {
		recurErase(root_);
		root_ = shm_nullptr;
		size_ = 0;
	}

protected:
	rbtree_node_ptr root_;
	size_t size_;
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
			root_ = new_node;
		} else if (old_node == old_node->parent->left_child) {
			old_node->parent->left_child = new_node;
		} else {
			old_node->parent->right_child = new_node;
		}

		if (new_node != shm_nullptr) {
			new_node->parent = old_node->parent;
		}
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

	void recurErase(rbtree_node_ptr& x) {
		if (x != shm_nullptr) {
			recurErase(x->left_child);
			recurErase(x->right_child);
			deleteNode(x);
		}
	}

	rbtree_node_ptr rbtree_lookup_key(const Key& key) {
		auto n = root_;
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
	rbtree_node_ptr rbtree_remove(rbtree_node_ptr node) {
		if (node == shm_nullptr) {
			return shm_nullptr;
		}

		// 如果待删除的节点有两个孩子需要转换成只有一个孩子，方法是找一个相邻的替换
		if (node->left_child != shm_nullptr && node->right_child != shm_nullptr) {
			auto k = node->left_child;

			while (k->right_child != shm_nullptr) {
				k = k->right_child;
			}

			// k是node左子树中最大的那一个，仍然比node小
			// node与k交换之后不会破坏二叉查找树的任何特性
			// 交换完成之后，node位置变了

			//
			// 此处还可以优化一下性能，其实修改几个指针就可以了
			//
			std::swap(node->value, k->value);
			std::swap(node, k);
		}

		// 现在node最多只会有一个孩子
		auto replacement = node->right_child != shm_nullptr ? node->right_child : node->left_child;

		if (replacement != shm_nullptr) {
			// 待删除节点只有一个孩子
			// 把node用replacement替换掉
			transplant(node, replacement);

			if (color(node) == RBTREE_NODE_BLACK) {
				repair_after_remove(replacement);
			}
		} else if (node->parent == shm_nullptr) {
			// 根节点
			root_ = shm_nullptr;
		} else {
			//无任何孩子节点

			if (color(node) == RBTREE_NODE_BLACK)
				repair_after_remove(node);

			if (node->parent != shm_nullptr) {
				if (node == node->parent->left_child)
					node->parent->left_child = shm_nullptr;
				else if (node == node->parent->right_child)
					node->parent->right_child = shm_nullptr;
				node->parent = shm_nullptr;
			}
		}

		// auto tmp = node.Ptr();
		deleteNode(node);
		--size_;
		return replacement;
	}

	rbtree_node_ptr rbtree_first() const {
		auto n = root_;
		if (n == shm_nullptr) {
			return shm_nullptr;
		}

		while (n->left_child != shm_nullptr) {
			n = n->left_child;
		}

		return n;
	}

	rbtree_node_ptr rbtree_last() {
		auto n = root_;
		if (n == shm_nullptr) {
			return shm_nullptr;
		}

		while (n->right_child != shm_nullptr) {
			n = n->right_child;
		}

		return n;
	}
};

} // namespace smd
