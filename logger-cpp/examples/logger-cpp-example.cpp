#include "../src/logger.hh"

#include <iostream>
#include <thread>

void test_console_logger()
{
    LogInstance().EnableModule(LogFlags::MODULE1);
    LOG(LogFlags::MODULE1, Level::Trace, "testing-Trace\n");
    LOG(LogFlags::MODULE1, Level::Debug, "testing-Debug\n");
    LOG(LogFlags::MODULE1, Level::Info, "testing-info\n");
    LOG(LogFlags::MODULE1, Level::Warning, "testing-Warning\n");
    LOG(LogFlags::MODULE1, Level::Error, "testing-Error\n");
    LOG(LogFlags::MODULE1, Level::None, "testing-None\n");

    LogInstance().set_ModuleLevel(LogFlags::MODULE1, Level::Debug);

    LOG(LogFlags::MODULE1, Level::Debug, "testing-Debug-agent\n");
}

void test_define_logger()
{
    LogInstance().EnableModule(LogFlags::MODULE2);
#define LOG_MODULE_2(level, ...) LOG(LogFlags::MODULE2, level, __VA_ARGS__)
    LOG_MODULE_2(Level::Trace, "testing-Debug\n");
    LogInstance().set_ModuleLevel(LogFlags::MODULE2, Level::Trace);
    LOG_MODULE_2(Level::Trace, "testing-Debug-agent\n");
#undef LOG_MODULE_2
}

int main(int argc, char *argv[])
{
    LogInstance().m_print_to_console = true;
    LogInstance().m_log_timestamps = true;
    test_console_logger();
    test_define_logger();
}