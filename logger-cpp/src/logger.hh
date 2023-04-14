
#ifndef __LAL_LOGGER_H__
#define __LAL_LOGGER_H__

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <memory>

namespace LALLog
{
enum class Level {
    Trace = 0,
    Debug,
    Info, // Default
    Warning,
    Error,
    None, // Internal use only
};

enum LogFlags : uint32_t {
    NONE = 0,
    MODULE1 = (1 << 0),
    MODULE2 = (1 << 1),
    ALL = ~(uint32_t)0,
};

constexpr auto DEFAULT_LEVEL = Level::Info;

class Logger
{
public:
    bool m_print_to_console = false;
    bool m_log_timestamps = false;

public:
    bool is_Enabled() const
    {
        std::lock_guard<std::mutex>(get_mutex());
        return m_print_to_console;
    }

    bool is_ModuleLevel(LogFlags module, Level level) const;
    bool set_ModuleLevel(const LALLog::LogFlags module, const LALLog::Level level);
    Level get_ModuleLevel() const
    {
        return m_log_level.load();
    }

    void EnableModule(LALLog::LogFlags module)
    {
        m_module |= module;
    }
    void DisableModule(LALLog::LogFlags module)
    {
        m_module &= ~module;
    }
    std::mutex &get_mutex()
    {
        return m_lock_;
    }
    void LogPrintStr(const std::string &str,
                     const std::string &logging_function,
                     const std::string &source_file, int source_line,
                     LALLog::LogFlags module, LALLog::Level level);

private:
    bool is_Module(LALLog::LogFlags module) const;
    std::string LogTimestampStr(const std::string &str);

private:
    std::mutex m_lock_;
    std::atomic<bool> m_started_new_line{true};
    std::atomic<uint32_t> m_module{0};
    std::unordered_map<LogFlags, Level> m_module_log_levels;
    std::atomic<Level> m_log_level{DEFAULT_LEVEL};
};

}; // namespace LALLog

LALLog::Logger &LogInstance();

/** Return true if log accepts specified module, at the specified level. */
static inline bool is_AcceptModule(LALLog::LogFlags module, LALLog::Level level)
{
    return LogInstance().is_ModuleLevel(module, level);
}

template <typename... Args>
static std::string str_format(const std::string &format, Args... args)
{
    auto size_buf = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
    std::unique_ptr<char[]> buf(new (std::nothrow) char[size_buf]);
    if (!buf)
        return std::string("");
    std::snprintf(buf.get(), size_buf, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size_buf - 1);
}

template <typename... Args>
static inline void LogPrintf_(const std::string &logging_function, const std::string &source_file,
                              const int source_line, const LALLog::LogFlags flag, const LALLog::Level level,
                              const char *fmt, const Args &... args)
{
    if (LogInstance().is_Enabled()) {
        std::string log_msg;
        log_msg = str_format(fmt, args...);
        LogInstance().LogPrintStr(log_msg, logging_function, source_file, source_line, flag, level);
    }
}

#define LogPrintLevel_(module, level, ...) LogPrintf_(__func__, __FILE__, __LINE__, module, level, __VA_ARGS__)

#define LOG(module, level, ...)                         \
    do {                                                \
        if (is_AcceptModule((module), (level))) {       \
            LogPrintLevel_(module, level, __VA_ARGS__); \
        }                                               \
    } while (0)

using namespace LALLog;
#endif /* __LAL_LOGGER_H__ */