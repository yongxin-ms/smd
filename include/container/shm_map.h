#pragma once
#include "pair.h"
#include "shm_obj.h"

namespace smd {

enum rbtree_node_color : bool {
	_red   = true,
	_black = false,
};

struct rbtree_node_base {
	typedef rbtree_node_color color_type;

	color_type		  color;
	rbtree_node_base* parent;
	rbtree_node_base* left;
	rbtree_node_base* right;

	rbtree_node_base()
		: color(_red)
		, parent(nullptr)
		, left(nullptr)
		, right(nullptr) {}
};

rbtree_node_base* RBTreeIncrement(rbtree_node_base* p) {
	if (p->right != nullptr) {
		p = p->right;
		while (p->left != nullptr)
			p = p->left;
	} else {
		rbtree_node_base* y = p->parent;
		while (p == y->right) {
			p = y;
			y = y->parent;
		}

		if (p->right != y)
			p = y;
	}
	return p;
}

rbtree_node_base* RBTreeDecrement(rbtree_node_base* p) {
	if (p->color == _red && p->parent->parent == p)
		p = p->right;
	else if (p->left != nullptr) {
		p = p->left;
		while (p->right != nullptr)
			p = p->right;
	} else {
		rbtree_node_base* y = p->parent;
		while (p == y->left) {
			p = y;
			y = y->parent;
		}
		p = y;
	}
	return p;
}

rbtree_node_base* RBTreeMinimum(rbtree_node_base* p) {
	while (p->left != nullptr)
		p = p->left;
	return p;
}

rbtree_node_base* RBTreeMaximum(rbtree_node_base* p) {
	while (p->right != nullptr)
		p = p->right;
	return p;
}

template <typename Value>
struct rbtree_node : public rbtree_node_base {
	Value value;
	rbtree_node(const Value& val)
		: rbtree_node_base()
		, value(val) {}
};

template <typename T, typename Pointer, typename Reference>
struct rbtree_iterator {
	typedef rbtree_iterator<T, Pointer, Reference> this_type;
	typedef rbtree_iterator<T, T*, T&>			   iterator;
	typedef rbtree_iterator<T, const T*, const T&> const_iterator;

	typedef ptrdiff_t		 difference_type;
	typedef T				 value_type;
	typedef rbtree_node_base base_node_type;
	typedef rbtree_node<T>	 node_type;
	typedef Pointer			 pointer;
	typedef Reference		 reference;

	node_type* p;

	rbtree_iterator()
		: p(nullptr) {}
	rbtree_iterator(node_type* pNode)
		: p(pNode) {}
	rbtree_iterator(const this_type& x)
		: p(x.p) {}

	reference operator*() const { return p->value; }
	pointer	  operator->() const { return &(p->value); }

	rbtree_iterator& operator++() {
		p = static_cast<node_type*>(RBTreeIncrement(p));
		return *this;
	}

	rbtree_iterator operator++(int) {
		this_type tmp(*this);
		p = static_cast<node_type*>(RBTreeIncrement(p));
		return tmp;
	}

	rbtree_iterator& operator--() {
		p = static_cast<node_type*>(RBTreeDecrement(p));
		return *this;
	}

	rbtree_iterator operator--(int) {
		this_type tmp(*this);
		p = static_cast<node_type*>(RBTreeDecrement(p));
		return *this;
	}

	bool operator==(const this_type& x) { return p == x.p; }
	bool operator!=(const this_type& x) { return p != x.p; }
};

template <typename Key, typename Value, typename Compare>
class rb_tree : public ShmObj {
public:
	typedef rbtree_node_color color_type;
	typedef ptrdiff_t		  difference_type;
	typedef size_t			  size_type;
	typedef Key				  key_type;
	typedef pair<Key, Value>  value_type;
	typedef value_type*		  pointer;
	typedef value_type&		  reference;
	typedef const value_type* const_pointer;
	typedef const value_type& const_reference;

	typedef rbtree_node<value_type> rbtreeNode;
	typedef rbtreeNode*				rbtree_node_ptr;

public:
	typedef rbtree_iterator<value_type, pointer, reference>				iterator;
	typedef rbtree_iterator<value_type, const_pointer, const_reference> const_iterator;

protected:
	rbtree_node_ptr header;
	size_type		node_count;
	Compare			key_compare;

protected:
	rbtree_node_ptr createNode(const value_type& val) { return m_alloc.New<rbtreeNode>(val); }
	void			deleteNode(rbtree_node_ptr p) { m_alloc.Delete(p); }

public:
	rb_tree(Alloc& alloc, const value_type& dummy)
		: ShmObj(alloc)
		, node_count(0) {
		header		  = createNode(dummy);
		leftmost()	  = header;
		rightmost()	  = header;
		color(header) = _red;
		root()		  = 0;
	}

	rb_tree(const rb_tree<Key, Value, Compare>& r)
		: ShmObj(r.m_alloc)
		, node_count(r.size()) {
		header		  = createNode(r.header->value);
		leftmost()	  = header;
		rightmost()	  = header;
		color(header) = _red;
		root()		  = 0;

		for (auto it = r.begin(); it != r.end(); ++it) {
			insert_unique(make_pair(it->first, it->second));
		}
	}

	rb_tree& operator=(const rb_tree<Key, Value, Compare>& r) {
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
	rbtree_node_ptr& root() const { return (rbtree_node_ptr&)header->parent; }
	rbtree_node_ptr& leftmost() const { return (rbtree_node_ptr&)header->left; }
	rbtree_node_ptr& rightmost() const { return (rbtree_node_ptr&)header->right; }

	static rbtree_node_ptr& left(rbtree_node_ptr x) { return (rbtree_node_ptr&)x->left; }
	static rbtree_node_ptr& right(rbtree_node_ptr x) { return (rbtree_node_ptr&)x->right; }
	static rbtree_node_ptr& parent(rbtree_node_ptr x) { return (rbtree_node_ptr&)x->parent; }
	static reference		value(rbtree_node_ptr x) { return x->value; }
	static const key_type&	key(rbtree_node_ptr x) { return (value(x)).first; }
	static color_type&		color(rbtree_node_ptr x) { return x->color; }

public:
	iterator	   begin() { return iterator(leftmost()); }
	const_iterator begin() const { return const_iterator(leftmost()); }

	iterator	   end() { return iterator(header); }
	const_iterator end() const { return const_iterator(header); }

	size_type size() const { return node_count; }
	size_type max_size() const { return size_type(-1); }

	bool empty() const { return node_count == 0; }

protected:
	iterator __insert(rbtree_node_ptr x, rbtree_node_ptr y, const value_type& val) {
		rbtree_node_ptr z;
		if (y == header || key_compare(val.first, key(y))) {
			z		= createNode(val);
			left(y) = z;
			if (y == header) {
				root()		= z;
				rightmost() = z;
			} else if (y == leftmost()) {
				leftmost() = z;
			}

		} else {
			z		 = createNode(val);
			right(y) = z;
			if (rightmost() == y) {
				rightmost() = z;
			}
		}

		parent(z) = y;
		left(z)	  = 0;
		right(z)  = 0;

		rb_tree_rebalance(z, parent(header));
		++node_count;
		return iterator(z);
	}

	void rb_tree_rebalance(rbtree_node_ptr z, rbtree_node_ptr& root) {
		z->color = _red;
		while (z != root && z->parent->color == _red) {
			if (z->parent == ((rbtree_node_ptr)(z->parent->parent->left))) {
				rbtree_node_ptr s = (rbtree_node_ptr)z->parent->parent->right;
				if (s != 0 && s->color == _red) {
					s->color				 = _black;
					z->parent->color		 = _black;
					z->parent->parent->color = _red;
					z						 = (rbtree_node_ptr)z->parent->parent;

				} else {
					if (z == (rbtree_node_ptr)z->parent->right) {
						z = (rbtree_node_ptr)z->parent;
						rb_tree_rotate_left(z, root);
					}
					z->parent->parent->color = _red;
					z->parent->color		 = _black;
					rb_tree_rotate_right((rbtree_node_ptr)z->parent->parent, root);
				}

			} else {
				rbtree_node_ptr s = (rbtree_node_ptr)z->parent->parent->left;
				if (s != 0 && s->color == _red) {
					s->color				 = _black;
					z->parent->color		 = _black;
					z->parent->parent->color = _red;
					z						 = (rbtree_node_ptr)z->parent->parent;
				} else {
					if (z == (rbtree_node_ptr)z->parent->left) {
						z = (rbtree_node_ptr)z->parent;
						rb_tree_rotate_right(z, root);
					}
					z->parent->parent->color = _red;
					z->parent->color		 = _black;
					rb_tree_rotate_left((rbtree_node_ptr)z->parent->parent, root);
				}
			}
		}
		root->color = _black;
	}

	void rb_tree_rotate_left(rbtree_node_ptr x, rbtree_node_ptr& root) {
		auto y	 = right(x);
		right(x) = left(y);
		if (y->left != 0)
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
		left(y)	  = x;
	}

	void rb_tree_rotate_right(rbtree_node_ptr x, rbtree_node_ptr& root) {
		auto y	= left(x);
		left(x) = right(y);
		if (y->right != 0)
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
		right(y)  = x;
	}

	void recurErase(rbtree_node_ptr x) {
		if (x != 0) {
			recurErase(left(x));
			recurErase(right(x));
			deleteNode(x);
		}
	}

	rbtree_node_ptr rb_tree_rebalance_for_erase(rbtree_node_ptr z, rbtree_node_ptr& root,
		rbtree_node_ptr& leftmost, rbtree_node_ptr& rightmost) {
		rbtree_node_ptr y = z;	  //实际删除的节点
		rbtree_node_ptr x = 0;	  //替代z(y)的节点
		rbtree_node_ptr x_parent; //删除之后 x 的父节点

		if (left(y) == 0) //这里帮找实际需要删除的y节点  以及 y的子节点 x(上位y)
		{
			x = right(y);
		} else if (right(y) == 0) {
			x = left(y);
		} else {
			y = right(y);
			while (left(y) != 0)
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

			if (x != 0)
				parent(x) = parent(z);

			if (z == leftmost) {
				if (right(z) == 0)
					leftmost = parent(z);
				else
					leftmost = (rbtree_node_ptr)(RBTreeMinimum(x));
			}

			if (z == rightmost) {
				if (z->left == 0)
					rightmost = parent(z);
				else
					rightmost = (rbtree_node_ptr)(RBTreeMaximum(x));
			}
		} else {
			left(y)			= left(z);
			parent(left(z)) = y;
			if (right(z) != y) {
				x_parent = parent(y);
				if (x != 0)
					parent(x) = parent(y);
				left(parent(y)) = x;

				right(y)		 = right(z);
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

			swap(y->color, z->color);
			y = z;
		}

		// y： 待删节点   x： 替换y的节点(鸠占鹊巢),此时x也成为标记节点   x_parent: x的父节点
		if (y->color != _red) {
			while (x != root && (x == 0 || x->color == _black)) //为双黑色节点
			{
				if (x == left(x_parent)) {
					rbtree_node_ptr w = right(x_parent);
					if (w->color == _red) {
						w->color		= _black;
						x_parent->color = _red;
						rb_tree_rotate_left(x_parent, root);
						w = right(x_parent);
					}
					if ((left(w) == 0 || color(left(w)) == _black) &&
						(right(w) == 0 || color(right(w)) == _black)) {
						w->color = _red;
						x		 = x_parent;
						x_parent = parent(x_parent);
					} else {
						if (right(w) == 0 || color(right(w)) == _black) {
							if (left(w) != 0)
								color(left(w)) = _black;
							color(right(w)) = _red;
							rb_tree_rotate_right(w, root);
							w = right(x_parent);
						}
						w->color		= x_parent->color;
						x_parent->color = _black;
						if (w->right != 0)
							color(right(w)) = _black;
						rb_tree_rotate_left(x_parent, root);
						break;
					}

				} else {
					rbtree_node_ptr w = left(x_parent);
					if (w->color == _red) {
						w->color		= _black;
						x_parent->color = _red;
						rb_tree_rotate_right(x_parent, root);
						w = left(x_parent);
					}

					if ((left(w) == 0 || color(left(w)) == _black) &&
						(right(w) == 0 || color(right(w)) == _black)) {
						w->color = _red;
						x		 = x_parent;
						x_parent = parent(x_parent);
					} else {
						if (left(w) == 0 || color(left(w)) == _black) {
							if (right(w) != 0)
								color(right(w)) = _black;
							w->color = _red;
							rb_tree_rotate_left(w, root);
							w = left(x_parent);
						}
						w->color		= x_parent->color;
						x_parent->color = _black;
						if (w->left != 0)
							color(left(w)) = _black;
						rb_tree_rotate_right(x_parent, root);
						break;
					}
				}
			}
			if (x != 0)
				x->color = _black;
		}

		return y;
	}

	rbtree_node_ptr findRBTree(const key_type& k, bool& isFind) {
		auto res = parent(header);
		isFind	 = false;
		while (res != 0) {
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
	pair<iterator, bool> insert_unique(const value_type& val) {
		auto p	 = header;
		auto x	 = root();
		bool res = true;
		while (x != 0) {
			p	= x;
			res = key_compare(val.first, key(x));
			if (res) {
				x = left(x);
			} else {
				x = right(x);
			}
		}

		//尚未有一个节点，此时根本没有进入while循环
		if (p == header) {
			return pair<iterator, bool>(__insert(x, p, val), true);
		}

		auto j = iterator(p);
		if (res) {
			if (j == begin()) {
				return pair<iterator, bool>(__insert(x, p, val), true);
			} else {
				--j;
			}
		}

		if (key_compare(key(j.p), val.first)) {
			return pair<iterator, bool>(__insert(x, p, val), true);
		}

		return pair<iterator, bool>(j, false);
	}

	void clear() {
		recurErase(root());
		left(header)   = header;
		right(header)  = header;
		parent(header) = 0;
	}

	iterator erase(iterator position) {
		auto pos_erase = position++;
		auto to_be_delete =
			rb_tree_rebalance_for_erase(pos_erase.p, root(), leftmost(), rightmost());
		deleteNode(to_be_delete);
		--node_count;
		return position;
	}

	iterator find(const key_type& k) {
		bool isFind = false;
		auto res	= findRBTree(k, isFind);
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
	typedef typename rb_tree<Key, Value, Compare>::iterator	  iterator;

	explicit ShmMap(Alloc& alloc, const valueType& dummy)
		: m_tree(alloc, dummy) {}

	ShmMap(const ShmMap<Key, Value, Compare>& r)
		: m_tree(r.m_tree) {}

	ShmMap& operator=(const ShmMap<Key, Value, Compare>& r) {
		if (this != &r) {
			ShmMap(r).swap(*this);
		}
		return *this;
	}

	pair<iterator, bool> insert(const valueType& v) { return m_tree.insert_unique(v); }

	bool	 empty() const { return m_tree.empty(); }
	size_t	 size() const { return m_tree.size(); }
	void	 clear() { m_tree.clear(); }
	iterator begin() { return m_tree.begin(); }
	iterator end() { return m_tree.end(); }
	iterator find(const Key& k) { return m_tree.find(k); }
	iterator erase(iterator it) { return m_tree.erase(it); }

private:
	rb_tree<Key, Value, Compare> m_tree;
};

} // namespace smd
