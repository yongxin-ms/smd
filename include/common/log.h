#pragma once
#include <functional>
#include <stdarg.h>

namespace smd {

class Log {
public:
	enum class LogLevel {
		kError = 1,
		kWarning,
		kInfo,
		kDebug,
	};

	void SetLogHandler(std::function<void(LogLevel, const char*)> f) { log_func_ = f; }
	void SetLogLevel(LogLevel lv) { log_level_ = lv; }

	void DoLog(LogLevel lv, const char* fmt, ...) {
		if (lv <= log_level_ && log_func_) {
			char buf[4096];
			va_list args;
			va_start(args, fmt);
			vsnprintf(buf, sizeof(buf) - 1, fmt, args);
			va_end(args);
			log_func_(lv, buf);
		}
	}

private:
	LogLevel log_level_ = LogLevel::kDebug;
	std::function<void(LogLevel, const char*)> log_func_ = nullptr;
};

} // namespace smd
