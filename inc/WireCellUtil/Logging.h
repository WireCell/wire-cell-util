#ifndef WIRECELL_LOGGING
#define WIRECELL_LOGGING


// SPDLOG_LOGGER_DEBUG() and SPDLOG_LOGGER_TRACE can be used to wrap
// very verbose messages and they can be deactivated at compile time
// so as to not suffer performance slowdowns.  Of course, do not put
// code with side effects inside these macros.

// Eventually set this via build configuration to DEBUG or maybe INFO.
// For development, we keep to trace although default set in wire-cell
// CLI are higher.
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include <string>

namespace WireCell {

    namespace Log {

        typedef std::shared_ptr<spdlog::logger> logptr_t;
        typedef std::shared_ptr<spdlog::sinks::sink> sinkptr_t;

        // WCT maintains a collection of sinks associated with all
        // loggers created through this API.  No sinks are added by
        // default.  The WCT application should add some if output is
        // wanted.  Note, all loggers made from here go to ALL sinks
        // added.  If unique loggers->sinks mapping is needed,
        // directly use calls in the spdlog:: namespace.
        void add_sink(sinkptr_t sink, std::string level="");

        // Add a log file sink with optional level.
        void add_file(std::string filename, std::string level="");

        // Add a standard out console sink with optional level.
        void add_stdout(bool color=true, std::string level="");

        // Add a standard err console sink with optional level.
        void add_stderr(bool color=true, std::string level="");

        // Get/make a logger by name.  If a logger by the name is not
        // yet existing then it will be created and attached to all
        // sinks that have been added prior to the call.  WCT
        // components are encouraged to may make unique loggers with
        // some short name related to the component type/name and hold
        // on to them for use in their code.  
        logptr_t logger(std::string name);

        // Set log level.  If which is empty the set level of logs.
        // Otherwise, set the given logger.
        void set_level(std::string level, std::string which="");

        // Set logging pattern.  If which is empty then
        // set pattern of all sinks.  Otherwise, set the given logger.
        void set_pattern(std::string pattern, std::string which="");
    }

}

#endif
