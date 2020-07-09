#pragma once
#include <string>

namespace smd {

class BaseObj {
public:
	virtual void serialize(std::string& to) = 0;
	virtual void deserialize(const std::string& from, size_t& pos) = 0;
};
} // namespace smd
