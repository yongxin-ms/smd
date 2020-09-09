#pragma once

namespace smd {

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

template <class T>
int64_t compare(const T& x, const T& y) {
	return x - y;
}

template <>
int64_t compare(const uint64_t& x, const uint64_t& y) {
	return x - y;
}

} // namespace smd
