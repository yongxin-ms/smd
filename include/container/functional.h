#pragma once

namespace smd {
//************ [swap] ***************
template <class T>
void swap(T& a, T& b) {
	T temp = a;
	a = b;
	b = temp;
}

//********** [less] ****************
template <class T>
struct less {
	typedef T first_argument_type;
	typedef T second_argument_type;
	typedef bool result_type;

	result_type operator()(const first_argument_type& x, const second_argument_type& y) {
		return x < y;
	}
};

//********** [equal_to] ****************
template <class T>
struct equal_to {
	typedef T first_argument_type;
	typedef T second_argument_type;
	typedef bool result_type;

	result_type operator()(const first_argument_type& x, const second_argument_type& y) {
		return x == y;
	}
};
} // namespace smd
