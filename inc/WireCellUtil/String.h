#ifndef WIRECELLUTIL_STRING
#define WIRECELLUTIL_STRING

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <vector>
#include <string>
#include <map>
#include <sstream>

namespace WireCell {

    namespace String {

        std::vector<std::string> split(const std::string& in, const std::string& delim=":");

        std::pair<std::string,std::string> parse_pair(const std::string& in, const std::string& delim=":");

        // format_flatten converts from "%"-list to variadic function call.
        template<typename TYPE>
        boost::format format_flatten(boost::format f, TYPE obj) {
            return f % obj;
        }
        template<typename TYPE, typename... MORE>
        boost::format format_flatten(boost::format start, TYPE o, MORE... objs) {
            auto next = start % o;
            return format_flatten(next, objs...);
        }
        inline boost::format format_flatten(boost::format f) {
            return f;
        }
        
        /** The format() function provides an sprintf() like function.
         It's a wrapper on boost::format() but returns a string
         instead of a stream and has function calling semantics
         instead of the "%" list.  Use like:

         int a=42;
         std::string foo = "bar";
         std::string msg = format("the answer to %s is %d", foo, a);
        */
        
        template<typename... TYPES>
        std::string format(const std::string& form, TYPES... objs) {
            auto final = format_flatten(boost::format(form), objs...);
            std::stringstream ss;
            ss << final;
            return ss.str();
        }


        // Turn some type into a string via streaming.
        template<typename T>
        std::string stringify(T obj) {
            std::stringstream ss;
            ss << obj;
            return ss.str();
        }

    }
}

#endif
