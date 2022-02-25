#pragma once
#include <functional>
#include <stdarg.h>

#define SMD_LOG_DEBUG(fmt, ...)                                                                                        \
	if (smd::g_log != nullptr)                                                                                         \
		smd::g_log->DoLog(smd::Log::LogLevel::kDebug, "[%s:%d %s()] " fmt, smd::Log::GetFileName(__FILE__), __LINE__,  \
						  __func__, ##__VA_ARGS__);
#define SMD_LOG_INFO(fmt, ...)                                                                                         \
	if (smd::g_log != nullptr)                                                                                         \
		smd::g_log->DoLog(smd::Log::LogLevel::kInfo, "[%s:%d %s()] " fmt, smd::Log::GetFileName(__FILE__), __LINE__,   \
						  __func__, ##__VA_ARGS__);
#define SMD_LOG_WARN(fmt, ...)                                                                                         \
	if (smd::g_log != nullptr)                                                                                         \
		smd::g_log->DoLog(smd::Log::LogLevel::kWarning, "[%s:%d %s()] " fmt, smd::Log::GetFileName(__FILE__),          \
						  __LINE__, __func__, ##__VA_ARGS__);
#define SMD_LOG_ERROR(fmt, ...)                                                                                        \
	if (smd::g_log != nullptr)                                                                                         \
		smd::g_log->DoLog(smd::Log::LogLevel::kError, "[%s:%d %s()] " fmt, smd::Log::GetFileName(__FILE__), __LINE__,  \
						  __func__, ##__VA_ARGS__);

namespace smd {

class Log {
public:
	enum class LogLevel {
		kError = 1,
		kWarning,
		kInfo,
		kDebug,
	};

	using LOG_FUNC = std::function<void(LogLevel, const char*)>;
	Log(LOG_FUNC&& func, LogLevel lv)
		: m_log_func(std::move(func))
		, m_log_level(lv) {}

	void SetLogLevel(LogLevel lv) {
		m_log_level = lv;
	}

	void DoLog(LogLevel lv, const char* fmt, ...) {
		if (lv <= m_log_level) {
			char buf[1024];
			va_list args;
			va_start(args, fmt);
			std::vsnprintf(buf, sizeof(buf) - 1, fmt, args);
			va_end(args);
			m_log_func(lv, buf);
		}
	}

	static const char* GetFileName(const char* file) {
		auto i = strrchr(file, '/');
		if (i != nullptr)
			return i + 1;
		else
			return file;
	}

private:
	LOG_FUNC m_log_func;
	LogLevel m_log_level = LogLevel::kDebug;
};

static Log* g_log = nullptr;

static void SetLogHandler(Log::LOG_FUNC&& func, Log::LogLevel log_level) {
	if (g_log != nullptr) {
		delete g_log;
		g_log = nullptr;
	}

	g_log = new Log(std::move(func), log_level);
}

} // namespace smd
