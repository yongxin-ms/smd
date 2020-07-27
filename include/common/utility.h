#pragma once
#include <stdint.h>

namespace smd {

class PrimeUtil {
public:
	uint64_t NextPrime(size_t n) const {
		auto i = 0;
		for (; i != PRIME_LIST_SIZE; ++i) {
			if (n > m_primeList[i])
				continue;
			else
				break;
		}

		i = (i == PRIME_LIST_SIZE ? PRIME_LIST_SIZE - 1 : i);
		return m_primeList[i];
	}

private:
	enum {
		PRIME_LIST_SIZE = 28,
	};

	const uint64_t m_primeList[PRIME_LIST_SIZE] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289,
		24593, 49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843,
		50331653, 100663319, 201326611, 402653189, 805306457, 1610612741, 3221225473, 4294967291};
};

class Utility {
public:
	static inline uint32_t IsPowOf2(uint32_t x) { return !(x & (x - 1)); }
	static inline uint32_t NextPowOf2(uint32_t x) {
		if (IsPowOf2(x))
			return x;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		return x + 1;
	}
};

} // namespace smd
