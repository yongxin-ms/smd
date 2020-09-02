#pragma once
#include "shm_pointer.h"

namespace smd {

enum rbtree_node_color : bool {
	_red = true,
	_black = false,
};

template <typename Value>
struct rbtree_node {
	typedef rbtree_node_color color_type;

	color_type color;
	ShmPointer<rbtree_node> parent;
	ShmPointer<rbtree_node> left;
	ShmPointer<rbtree_node> right;

	Value value;
	rbtree_node(const Value& val)
		: color(_red)
		, parent(shm_nullptr)
		, left(shm_nullptr)
		, right(shm_nullptr)
		, value(val) {}
};

template <typename Value>
ShmPointer<rbtree_node<Value>> RBTreeIncrement(ShmPointer<rbtree_node<Value>> p) {
	if (p->right != shm_nullptr) {
		p = p->right;
		while (p->left != shm_nullptr)
			p = p->left;
	} else {
		auto y = p->parent;
		while (p == y->right) {
			p = y;
			y = y->parent;
		}

		if (p->right != y)
			p = y;
	}
	return p;
}

template <typename Value>
ShmPointer<rbtree_node<Value>> RBTreeDecrement(ShmPointer<rbtree_node<Value>> p) {
	if (p->color == _red && p->parent->parent == p)
		p = p->right;
	else if (p->left != shm_nullptr) {
		p = p->left;
		while (p->right != shm_nullptr)
			p = p->right;
	} else {
		auto y = p->parent;
		while (p == y->left) {
			p = y;
			y = y->parent;
		}
		p = y;
	}
	return p;
}

template <typename Value>
ShmPointer<rbtree_node<Value>> RBTreeMinimum(ShmPointer<rbtree_node<Value>> p) {
	while (p->left != shm_nullptr)
		p = p->left;
	return p;
}

template <typename Value>
ShmPointer<rbtree_node<Value>> RBTreeMaximum(ShmPointer<rbtree_node<Value>> p) {
	while (p->right != shm_nullptr)
		p = p->right;
	return p;
}

template <typename T, typename Pointer, typename Reference>
struct rbtree_iterator {
	typedef rbtree_iterator<T, Pointer, Reference> this_type;

	ShmPointer<rbtree_node<T>> p;

	rbtree_iterator()
		: p(shm_nullptr) {}
	rbtree_iterator(ShmPointer<rbtree_node<T>> pNode)
		: p(pNode) {}
	rbtree_iterator(const this_type& x)
		: p(x.p) {}

	Reference operator*() const { return p->value; }
	Pointer operator->() const { return &(p->value); }

	rbtree_iterator& operator++() {
		p = RBTreeIncrement(p);
		return *this;
	}

	rbtree_iterator operator++(int) {
		this_type tmp(*this);
		p = RBTreeIncrement(p);
		return tmp;
	}

	rbtree_iterator& operator--() {
		p = RBTreeDecrement(p);
		return *this;
	}

	rbtree_iterator operator--(int) {
		this_type tmp(*this);
		p = RBTreeDecrement(p);
		return tmp;
	}

	bool operator==(const this_type& x) { return p == x.p; }
	bool operator!=(const this_type& x) { return p != x.p; }
};

template <typename Key, typename Value, typename Compare>
class rb_tree {
public:
	typedef rb_tree<Key, Value, Compare> this_type;
	typedef shm_pair<Key, Value> value_type;
	typedef ShmPointer<rbtree_node<value_type>> rbtree_node_ptr;
	typedef rbtree_iterator<value_type, value_type*, value_type&> iterator;
	typedef rbtree_iterator<value_type, const value_type*, const value_type&> const_iterator;

protected:
	rbtree_node_ptr header;
	size_t node_count;
	Compare key_compare;

protected:
	rbtree_node_ptr createNode(const value_type& val) {
		return g_alloc->New<rbtree_node<value_type>>(val);
	}
	void deleteNode(rbtree_node_ptr p) { g_alloc->Delete(p); }

public:
	rb_tree(const Compare& comp)
		: node_count(0) {
		header = createNode(value_type());
		leftmost() = header;
		rightmost() = header;
		color(header) = _red;
		root() = shm_nullptr;
	}

	rb_tree(const this_type& r)
		: node_count(r.size())
		, key_compare(r.key_compare) {
		header = createNode(r.header->value);
		leftmost() = header;
		rightmost() = header;
		color(header) = _red;
		root() = shm_nullptr;

		for (auto it = r.begin(); it != r.end(); ++it) {
			insert_unique(make_pair(it->first, it->second));
		}
	}

	this_type& operator=(const this_type& r) {
		if (this != &r) {
			rb_tree(r).swap(*this);
		}
		return *this;
	}

	~rb_tree() {
		clear();
		deleteNode(header);
	}

protected:
	rbtree_node_ptr& root() const { return header->parent; }
	rbtree_node_ptr& leftmost() const { return header->left; }
	rbtree_node_ptr& rightmost() const { return header->right; }

	static rbtree_node_ptr& left(rbtree_node_ptr x) { return x->left; }
	static rbtree_node_ptr& right(rbtree_node_ptr x) { return x->right; }
	static rbtree_node_ptr& parent(rbtree_node_ptr x) { return x->parent; }
	static value_type& value(rbtree_node_ptr x) { return x->value; }
	static const Key& key(rbtree_node_ptr x) { return (value(x)).first; }
	static rbtree_node_color& color(rbtree_node_ptr x) { return x->color; }

public:
	iterator begin() { return iterator(leftmost()); }
	const_iterator begin() const { return const_iterator(leftmost()); }
	iterator end() { return iterator(header); }
	const_iterator end() const { return const_iterator(header); }
	size_t size() const { return node_count; }
	size_t max_size() const { return size_t(-1); }
	bool empty() const { return node_count == 0; }

protected:
	iterator __insert(rbtree_node_ptr x, rbtree_node_ptr y, const value_type& val) {
		rbtree_node_ptr z;
		if (y == header || key_compare(val.first, key(y))) {
			z = createNode(val);
			left(y) = z;
			if (y == header) {
				root() = z;
				rightmost() = z;
			} else if (y == leftmost()) {
				leftmost() = z;
			}

		} else {
			z = createNode(val);
			right(y) = z;
			if (rightmost() == y) {
				rightmost() = z;
			}
		}

		parent(z) = y;
		left(z) = shm_nullptr;
		right(z) = shm_nullptr;

		rb_tree_rebalance(z, parent(header));
		++node_count;
		return iterator(z);
	}

	void rb_tree_rebalance(rbtree_node_ptr z, rbtree_node_ptr& root) {
		z->color = _red;
		while (z != root && z->parent->color == _red) {
			if (z->parent == z->parent->parent->left) {
				rbtree_node_ptr s = z->parent->parent->right;
				if (s != shm_nullptr && s->color == _red) {
					s->color = _black;
					z->parent->color = _black;
					z->parent->parent->color = _red;
					z = z->parent->parent;

				} else {
					if (z == z->parent->right) {
						z = z->parent;
						rb_tree_rotate_left(z, root);
					}
					z->parent->parent->color = _red;
					z->parent->color = _black;
					rb_tree_rotate_right(z->parent->parent, root);
				}

			} else {
				rbtree_node_ptr s = z->parent->parent->left;
				if (s != shm_nullptr && s->color == _red) {
					s->color = _black;
					z->parent->color = _black;
					z->parent->parent->color = _red;
					z = z->parent->parent;
				} else {
					if (z == z->parent->left) {
						z = z->parent;
						rb_tree_rotate_right(z, root);
					}
					z->parent->parent->color = _red;
					z->parent->color = _black;
					rb_tree_rotate_left(z->parent->parent, root);
				}
			}
		}
		root->color = _black;
	}

	void rb_tree_rotate_left(rbtree_node_ptr x, rbtree_node_ptr& root) {
		auto y = right(x);
		right(x) = left(y);
		if (y->left != shm_nullptr)
			parent(left(y)) = x;

		parent(y) = parent(x);
		if (x == root)
			root = y;
		else if (left(parent(x)) == x) {
			left(parent(x)) = y;
		} else {
			right(parent(x)) = y;
		}

		parent(x) = y;
		left(y) = x;
	}

	void rb_tree_rotate_right(rbtree_node_ptr x, rbtree_node_ptr& root) {
		auto y = left(x);
		left(x) = right(y);
		if (y->right != shm_nullptr)
			parent(right(y)) = x;

		parent(y) = parent(x);
		if (x == root)
			y = root;
		else if (left(parent(x)) == x) {
			left(parent(x)) = y;
		} else {
			right(parent(x)) = y;
		}

		parent(x) = y;
		right(y) = x;
	}

	void recurErase(rbtree_node_ptr x) {
		if (x != shm_nullptr) {
			recurErase(left(x));
			recurErase(right(x));
			deleteNode(x);
		}
	}

	rbtree_node_ptr rb_tree_rebalance_for_erase(rbtree_node_ptr z, rbtree_node_ptr& root,
		rbtree_node_ptr& leftmost, rbtree_node_ptr& rightmost) {
		rbtree_node_ptr y = z;	  //实际删除的节点
		rbtree_node_ptr x = shm_nullptr; //替代z(y)的节点
		rbtree_node_ptr x_parent; //删除之后 x 的父节点

		if (left(y) == shm_nullptr) //这里帮找实际需要删除的y节点  以及 y的子节点 x(上位y)
		{
			x = right(y);
		} else if (right(y) == shm_nullptr) {
			x = left(y);
		} else {
			y = right(y);
			while (left(y) != shm_nullptr)
				y = left(y);
			x = right(y);
		}

		if (y == z) {
			x_parent = parent(y);
			if (z == root) {
				root = x;
			} else if (z == left(parent(z))) {
				left(parent(z)) = x;
			} else {
				right(parent(z)) = x;
			}

			if (x != shm_nullptr)
				parent(x) = parent(z);

			if (z == leftmost) {
				if (right(z) == shm_nullptr)
					leftmost = parent(z);
				else
					leftmost = RBTreeMinimum(x);
			}

			if (z == rightmost) {
				if (z->left == shm_nullptr)
					rightmost = parent(z);
				else
					rightmost = RBTreeMaximum(x);
			}
		} else {
			left(y) = left(z);
			parent(left(z)) = y;
			if (right(z) != y) {
				x_parent = parent(y);
				if (x != shm_nullptr)
					parent(x) = parent(y);
				left(parent(y)) = x;

				right(y) = right(z);
				parent(right(z)) = y;
			} else {
				x_parent = y;
			}

			if (z == root) {
				root = y;
			} else if (z == left(parent(z))) {
				left(parent(z)) = y;
			} else {
				right(parent(z)) = y;
			}
			parent(y) = parent(z);

			std::swap(y->color, z->color);
			y = z;
		}

		// y： 待删节点   x： 替换y的节点(鸠占鹊巢),此时x也成为标记节点   x_parent: x的父节点
		if (y->color != _red) {
			while (x != root && (x == shm_nullptr || x->color == _black)) //为双黑色节点
			{
				if (x == left(x_parent)) {
					rbtree_node_ptr w = right(x_parent);
					if (w->color == _red) {
						w->color = _black;
						x_parent->color = _red;
						rb_tree_rotate_left(x_parent, root);
						w = right(x_parent);
					}
					if ((left(w) == shm_nullptr || color(left(w)) == _black) &&
						(right(w) == shm_nullptr || color(right(w)) == _black)) {
						w->color = _red;
						x = x_parent;
						x_parent = parent(x_parent);
					} else {
						if (right(w) == shm_nullptr || color(right(w)) == _black) {
							if (left(w) != shm_nullptr)
								color(left(w)) = _black;
							color(right(w)) = _red;
							rb_tree_rotate_right(w, root);
							w = right(x_parent);
						}
						w->color = x_parent->color;
						x_parent->color = _black;
						if (w->right != shm_nullptr)
							color(right(w)) = _black;
						rb_tree_rotate_left(x_parent, root);
						break;
					}

				} else {
					rbtree_node_ptr w = left(x_parent);
					if (w->color == _red) {
						w->color = _black;
						x_parent->color = _red;
						rb_tree_rotate_right(x_parent, root);
						w = left(x_parent);
					}

					if ((left(w) == shm_nullptr || color(left(w)) == _black) &&
						(right(w) == shm_nullptr || color(right(w)) == _black)) {
						w->color = _red;
						x = x_parent;
						x_parent = parent(x_parent);
					} else {
						if (left(w) == shm_nullptr || color(left(w)) == _black) {
							if (right(w) != shm_nullptr)
								color(right(w)) = _black;
							w->color = _red;
							rb_tree_rotate_left(w, root);
							w = left(x_parent);
						}
						w->color = x_parent->color;
						x_parent->color = _black;
						if (w->left != shm_nullptr)
							color(left(w)) = _black;
						rb_tree_rotate_right(x_parent, root);
						break;
					}
				}
			}
			if (x != shm_nullptr)
				x->color = _black;
		}

		return y;
	}

	rbtree_node_ptr findRBTree(const Key& k, bool& isFind) {
		auto res = parent(header);
		isFind = false;
		while (res != shm_nullptr) {
			if (key_compare(k, key(res))) {
				res = left(res);
			} else {
				if (key_compare(key(res), k)) {
					res = right(res);
				} else {
					isFind = true;
					break;
				}
			}
		}
		return res;
	}

public:
	shm_pair<iterator, bool> insert_unique(const value_type& val) {
		auto p = header;
		auto x = root();
		bool res = true;
		while (x != shm_nullptr) {
			p = x;
			res = key_compare(val.first, key(x));
			if (res) {
				x = left(x);
			} else {
				x = right(x);
			}
		}

		//尚未有一个节点，此时根本没有进入while循环
		if (p == header) {
			return shm_pair<iterator, bool>(__insert(x, p, val), true);
		}

		auto j = iterator(p);
		if (res) {
			if (j == begin()) {
				return shm_pair<iterator, bool>(__insert(x, p, val), true);
			} else {
				--j;
			}
		}

		if (key_compare(key(j.p), val.first)) {
			return shm_pair<iterator, bool>(__insert(x, p, val), true);
		}

		return shm_pair<iterator, bool>(j, false);
	}

	void clear() {
		recurErase(root());
		left(header) = header;
		right(header) = header;
		parent(header) = shm_nullptr;
		node_count = 0;
	}

	void erase(iterator position) {
		auto to_be_delete =
			rb_tree_rebalance_for_erase(position.p, root(), leftmost(), rightmost());
		deleteNode(to_be_delete);
		--node_count;
	}

	iterator find(const Key& k) {
		bool isFind = false;
		auto res = findRBTree(k, isFind);
		if (isFind) {
			return iterator(res);
		} else {
			return end();
		}
	}
};

template <typename Key, typename Value, typename Compare = less<Key>>
class ShmMap {
public:
	typedef typename rb_tree<Key, Value, Compare>::value_type valueType;
	typedef typename rb_tree<Key, Value, Compare>::iterator iterator;

	ShmMap()
		: m_tree(Compare()) {}
	ShmMap(const ShmMap<Key, Value, Compare>& r)
		: m_tree(r.m_tree) {}

	ShmMap& operator=(const ShmMap<Key, Value, Compare>& r) {
		if (this != &r) {
			ShmMap(r).swap(*this);
		}
		return *this;
	}

	shm_pair<iterator, bool> insert(const valueType& v) { return m_tree.insert_unique(v); }
	bool empty() const { return m_tree.empty(); }
	size_t size() const { return m_tree.size(); }
	void clear() { m_tree.clear(); }
	iterator begin() { return m_tree.begin(); }
	iterator end() { return m_tree.end(); }
	iterator find(const Key& k) { return m_tree.find(k); }
	void erase(iterator it) { m_tree.erase(it); }

private:
	rb_tree<Key, Value, Compare> m_tree;
};

} // namespace smd
