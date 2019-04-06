#include "WireCellUtil/Logging.h"

#include <string>

using namespace WireCell;

int main(int argc, char* argv[])
{
    std::string fname = argv[0];
    fname += ".log";
    spdlog::info("adding log file {}", fname);
    Log::add_file(fname);
    Log::add_stdout();

    Log::set_default("test_logging");
    auto l = Log::logger("test_logging");
    l->set_pattern("[%H:%M:%S.%03e] [%n:%L] %v");

    l->info("from my logger");
    spdlog::info("from default logger");

    SPDLOG_LOGGER_DEBUG(l, "log from debug CPP macro");
    SPDLOG_LOGGER_TRACE(l, "log from trace CPP macro, should not see by default");

    return 0;
}
