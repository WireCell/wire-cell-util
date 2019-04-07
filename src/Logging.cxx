#include "WireCellUtil/Logging.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/null_sink.h"

#include <vector>

using namespace WireCell;


void Log::add_sink(Log::sinkptr_t sink, std::string level)
{
    if (!level.empty()) {
        sink->set_level(spdlog::level::from_str(level));
    }
    spdlog::default_logger()->sinks().push_back(sink);
}
void Log::add_file(std::string filename, std::string level)
{
    auto s = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
    Log::add_sink(s, level);
}

void Log::add_stdout(bool color, std::string level)
{
    if (color) {
        auto s = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        Log::add_sink(s, level);
    }
    else {
        auto s = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        Log::add_sink(s, level);
    }
}
void Log::add_stderr(bool color, std::string level)
{
    if (color) {
        auto s = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        Log::add_sink(s, level);
    }
    else {
        auto s = std::make_shared<spdlog::sinks::stderr_sink_mt>();
        Log::add_sink(s, level);
    }
}

static
Log::logptr_t wct_base_logger()
{
    const std::string name = "wct";
    auto l = spdlog::get(name);
    if (!l) {
        std::vector<spdlog::sink_ptr> sv;
        l = std::make_shared<spdlog::logger>(name, sv.begin(), sv.end());
    }
    spdlog::set_default_logger(l);
    return l;
}

Log::logptr_t Log::logger(std::string name)
{
    wct_base_logger();          // make sure base logger is installed.
    auto l = spdlog::get(name);
    if (!l) {
        auto& sinks = spdlog::default_logger()->sinks();
        l = std::make_shared<spdlog::logger>(name,  sinks.begin(), sinks.end());

        // peak under the hood of spdlog.  We want shared loggers to
        // get configured with the default level.
        spdlog::details::registry::instance().initialize_logger(l);

    }
    return l;
}

void Log::set_level(std::string level, std::string which)
{
    auto lvl = spdlog::level::from_str(level);

    if (which.empty()) {
        spdlog::set_level(lvl);
        return;
    }
    logger(which)->set_level(lvl);
}
void Log::set_pattern(std::string pattern, std::string which)
{
    if (which.empty()) {
        spdlog::set_pattern(pattern);
        return;
    }
    logger(which)->set_pattern(pattern);
}
