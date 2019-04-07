#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Logging.h"

#include <string>

using namespace WireCell;

int main(int argc, char* argv[])
{
    std::string fname = argv[0];
    fname += ".log";

    auto l = Log::logger("test_logging");
    Assert(l != spdlog::default_logger());

    Log::add_stdout(true, "debug");

    spdlog::info("adding log file {}", fname);
    Log::add_file(fname, "error");

    auto b = Log::logger("before");

    Log::set_level("info");

    auto a = Log::logger("after");

    Log::set_level("debug", "special");

    auto s = Log::logger("special");

    l->set_pattern("[%H:%M:%S.%03e] [%n:%L] %v");

    l->error("error test logger");
    b->error("error other logger");
    a->error("error other logger");
    s->error("error other logger");
    spdlog::error("error default logger");

    l->info("info test logger");
    b->info("info other logger");
    a->info("info other logger");
    s->info("info other logger");
    spdlog::info("info default logger");

    l->debug("debug test logger");
    b->debug("debug other logger");
    a->debug("debug other logger");
    s->debug("debug other logger");
    spdlog::debug("debug default logger");


    //SPDLOG_LOGGER_DEBUG(l, "log from debug CPP macro");
    //SPDLOG_LOGGER_TRACE(l, "log from trace CPP macro, should not see by default");

    return 0;
}
