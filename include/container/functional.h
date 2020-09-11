#pragma once

namespace smd {

template <class T>
int64_t compare(const T& x, const T& y) {
	return x - y;
}

template <>
int64_t compare(const uint64_t& x, const uint64_t& y) {
	return x - y;
}

} // namespace smd
