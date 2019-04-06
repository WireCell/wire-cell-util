#include "WireCellUtil/Logging.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <vector>

using namespace WireCell;

std::vector<Log::sinkptr_t> s_sinks;

void Log::add_sink(Log::sinkptr_t sink)
{
    s_sinks.push_back(sink);
}
void Log::add_file(std::string filename)
{
    Log::add_sink(std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename));
}
void Log::add_stdout(bool color)
{
    if (color) {
        Log::add_sink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }
    else {
        Log::add_sink(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    }
}
void Log::add_stderr(bool color)
{
    if (color) {
        Log::add_sink(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
    }
    else {
        Log::add_sink(std::make_shared<spdlog::sinks::stderr_sink_mt>());
    }
}
Log::logptr_t Log::logger(std::string name)
{
    auto l = spdlog::get(name);
    if (!l) {
        l = std::make_shared<spdlog::logger>(name,  s_sinks.begin(), s_sinks.end());
    }
    return l;
}

Log::logptr_t Log::set_default(std::string name)
{
    auto l = Log::logger(name);
    spdlog::set_default_logger(l);
    return l;
}
    
void Log::defaults()
{
    add_stderr();
    auto l = set_default("log");
    
    l->set_pattern("[%H:%M:%S.%03e] %L [%=16n] %v");

}
