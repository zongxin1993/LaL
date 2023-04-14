#include "logger.hh"
#include <assert.h>

constexpr auto MAX_USER_SETABLE_SEVERITY_LEVEL{LALLog::Level::Info};

std::string LogModuleToStr(LALLog::LogFlags module)
{
    switch (module) {
    case LALLog::LogFlags::NONE:
        return "";
    case LALLog::LogFlags::MODULE1:
        return "MODULE1";
    case LALLog::LogFlags::MODULE2:
        return "MODULE2";
    case LALLog::LogFlags::ALL:
        return "All";
    };
    assert(false);
}

std::string LogLevelToStr(LALLog::Level level)
{
    switch (level) {
    case LALLog::Level::Trace:
        return "trace";
    case LALLog::Level::Debug:
        return "debug";
    case LALLog::Level::Info:
        return "info";
    case LALLog::Level::Warning:
        return "warning";
    case LALLog::Level::Error:
        return "error";
    case LALLog::Level::None:
        return "";
    }
    assert(false);
}

std::string LALLog::Logger::LogTimestampStr(const std::string &str)
{
    std::string res;
    if (!m_log_timestamps)
        return str;
    if (m_started_new_line) {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::nanoseconds d = now.time_since_epoch();
        std::chrono::milliseconds millsec = std::chrono::duration_cast<std::chrono::milliseconds>(d);
        time_t timestamp = std::chrono::system_clock::to_time_t(now);
        struct tm tLocal;
        localtime_r(&timestamp, &tLocal);
        res = str_format("%02d:%02d:%02d.%03d",
                         tLocal.tm_hour,
                         tLocal.tm_min,
                         tLocal.tm_sec,
                         (int)(millsec.count() % 1000));
        res += str;
    } else {
        res = str;
    }
    return res;
}

LALLog::Logger &LogInstance()
{
    /**
 * NOTE: the logger instances is leaked on exit. This is ugly, but will be
 * cleaned up by the OS/libc. Defining a logger as a global object doesn't work
 * since the order of destruction of static/global objects is undefined.
 * Consider if the logger gets destroyed, and then some later destructor calls
 * LogPrintf, maybe indirectly, and you get a core dump at shutdown trying to
 * access the logger. When the shutdown sequence is fully audited and tested,
 * explicit destruction of these objects can be implemented by changing this
 * from a raw pointer to a std::unique_ptr.
 * Since the ~Logger() destructor is never called, the Logger class and all
 * its subclasses must have implicitly-defined destructors.
 *
 */
    static LALLog::Logger *g_logger{new LALLog::Logger()};
    return *g_logger;
}

void LALLog::Logger::LogPrintStr(const std::string &str,
                                 const std::string &logging_function,
                                 const std::string &source_file, int source_line,
                                 LALLog::LogFlags module, LALLog::Level level)
{
    std::lock_guard<std::mutex>(get_mutex());

    std::string str_prefixed = (str);

    if ((module != LogFlags::NONE || level != Level::None) && m_started_new_line) {
        std::string s{"["};

        if (module != LogFlags::NONE) {
            s += LogModuleToStr(module);
        }

        if (module != LogFlags::NONE && level != Level::None) {
            // Only add separator if both flag and level are not NONE
            s += ":";
        }

        if (level != Level::None) {
            s += LogLevelToStr(level);
        }

        s += "] ";
        str_prefixed.insert(0, s);
    }

    str_prefixed = LogTimestampStr(str_prefixed);

    m_started_new_line = !str.empty() && str[str.size() - 1] == '\n';

    if (m_print_to_console) {
        // print to console
        fwrite(str_prefixed.data(), 1, str_prefixed.size(), stdout);
        fflush(stdout);
    }
}

bool LALLog::Logger::is_Module(LALLog::LogFlags module) const
{
    return (m_module.load(std::memory_order_relaxed) & module) != 0;
}

bool LALLog::Logger::is_ModuleLevel(LALLog::LogFlags module, LALLog::Level level) const
{
    if (level >= LALLog::Level::Warning) return true;

    if (!is_Module(module)) return false;

    std::lock_guard<std::mutex>(get_mutex());
    const auto it{m_module_log_levels.find(module)};
    bool res = level >= (it == m_module_log_levels.end() ? get_ModuleLevel() : it->second);

    return level >= (it == m_module_log_levels.end() ? get_ModuleLevel() : it->second);
}

bool LALLog::Logger::set_ModuleLevel(const LALLog::LogFlags module, const LALLog::Level level)
{
    if (level > MAX_USER_SETABLE_SEVERITY_LEVEL) return false;

    std::lock_guard<std::mutex>(get_mutex());
    m_module_log_levels[module] = level;
    return true;
}
