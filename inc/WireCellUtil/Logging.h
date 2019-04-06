#ifndef WIRECELL_LOGGING
#define WIRECELL_LOGGING


// SPDLOG_LOGGER_DEBUG() and SPDLOG_LOGGER_TRACE can be used to wrap
// very verbose messages and they can be deactivated at compile time
// so as to not suffer performance slowdowns.  Of course, do not put
// code with side effects inside these macros.

// Eventually set this via build configuration.  By default TRACE is
// turned off.
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

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
        void add_sink(sinkptr_t sink);

        // Add a log file sink.
        void add_file(std::string filename);

        // Add a standard out console sink.
        void add_stdout(bool color=true);

        // Add a standard err console sink.
        void add_stderr(bool color=true);

        // Get/make a logger by name.  If a logger by the name is not
        // yet existing then it will be created and attached to all
        // sinks that have been added prior to the call.  WCT
        // components are encouraged to may make unique loggers with
        // some short name related to the component type/name and hold
        // on to them for use in their code.  
        logptr_t logger(std::string name);

        // If a default logger is defined then bare spdlog::info(),
        // etc, functions will use it.
        logptr_t set_default(std::string name);

        // One stop shopping: this defines a color stderr logger
        // called "log" which is set as default.
        void defaults();

    }

}

#endif
