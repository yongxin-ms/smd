#pragma once
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <string.h>
#include <stdarg.h>

namespace Util {
class Random {
public:
	template <class T>
	static T RandomInt(T low, T high) {
		static std::random_device rd;
		static std::default_random_engine engine(rd());

		std::uniform_int_distribution<T> dis(0, high - low);
		T dice_roll = dis(engine) + low;
		return dice_roll;
	}
};

class Time {
public:
	static tm LocalTime(time_t t) {
		tm this_tm;
#ifdef _WIN32
		localtime_s(&this_tm, &t);
#else
		localtime_r(&t, &this_tm);
#endif // _WIN32
		return this_tm;
	}

	//把时间转换成字符串表示
	static std::string FormatDateTime(time_t t) {
		tm this_tm = LocalTime(t);
		char data[64] = {0};
		snprintf(data, sizeof(data) - 1, "%d-%02d-%02d %02d:%02d:%02d", this_tm.tm_year + 1900,
			this_tm.tm_mon + 1, this_tm.tm_mday, this_tm.tm_hour, this_tm.tm_min, this_tm.tm_sec);
		return data;
	}

	static time_t StrToDateTime(const char* input) {
		if (strlen(input) != strlen("1970-01-01 08:00:00"))
			return 0;
		tm this_tm = LocalTime(time(nullptr));
#ifdef _WIN32
		sscanf_s(input,
#else
		sscanf(input,
#endif
			"%d-%02d-%02d %02d:%02d:%02d", &this_tm.tm_year, &this_tm.tm_mon, &this_tm.tm_mday,
			&this_tm.tm_hour, &this_tm.tm_min, &this_tm.tm_sec);
		this_tm.tm_year -= 1900;
		this_tm.tm_mon -= 1;
		return mktime(&this_tm);
	}
};

class Text {
public:
	template <class T>
	static T _s2n(const char* s) {
		T n = T();
		if (s && *s) {
			bool negative = false;
			if (*s == '-') {
				negative = true;
				++s;
			}
			while (*s && *s >= '0' && *s <= '9') {
				n = n * 10 + (*s - '0');
				++s;
			}
			if (negative)
				n = (-1) * n;
		}
		return n;
	}

	static std::string Format(const char* fmt, ...) {
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf) - 1, fmt, args);
		va_end(args);
		return buf;
	}

	static int SafeStrcpy(char* dest, int size, const char* src) {
		int len = int(strlen(src));
		if (len >= size)
			return 0;
		memcpy(dest, src, len);
		dest[len] = 0;
		return len;
	}

	// 拆分字符串
	static size_t SplitStr(std::vector<std::string>& vec, const std::string& is, const char c) {
		vec.clear();

		std::string::size_type pos_find = is.find_first_of(c, 0);
		std::string::size_type pos_last_find = 0;
		while (std::string::npos != pos_find) {
			std::string::size_type num_char = pos_find - pos_last_find;
			vec.emplace_back(is.substr(pos_last_find, num_char));

			pos_last_find = pos_find + 1;
			pos_find = is.find_first_of(c, pos_last_find);
		}

		std::string::size_type num_char = is.length() - pos_last_find;
		if (num_char > is.length())
			num_char = is.length();

		const std::string& sub = is.substr(pos_last_find, num_char);
		vec.emplace_back(sub);

		return vec.size();
	};

	// 拆分用特殊字符分隔的整数字符串
	template <class T>
	static size_t SplitInt(std::vector<T>& vec, const std::string& is, const char c) {
		std::vector<std::string> vec_string;
		SplitStr(vec_string, is, c);

		vec.clear();
		for (size_t i = 0; i < vec_string.size(); i++) {
			const std::string& value = vec_string[i];
			vec.push_back(_s2n<T>(value.data()));
		}

		return vec.size();
	};

	// 拆分用特殊字符分隔的浮点字符串
	static size_t SplitFloat(std::vector<double>& vec, const std::string& is, const char c) {
		std::vector<std::string> vec_string;
		SplitStr(vec_string, is, c);

		vec.clear();
		for (size_t i = 0; i < vec_string.size(); i++) {
			const std::string& value = vec_string[i];
			vec.push_back(std::stod(value));
		}

		return vec.size();
	};

	// 拆分用特殊字符分隔的布尔字符串
	static size_t SplitBool(std::vector<bool>& vec, const std::string& is, const char c) {
		std::vector<std::string> vec_string;
		SplitStr(vec_string, is, c);

		vec.clear();
		for (size_t i = 0; i < vec_string.size(); i++) {
			const std::string& value = vec_string[i];
			vec.push_back(atoi(value.data()) != 0);
		}

		return vec.size();
	};
};
} // namespace Util
