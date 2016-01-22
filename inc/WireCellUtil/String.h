#ifndef WIRECELLUTIL_STRING
#define WIRECELLUTIL_STRING

#include <boost/algorithm/string.hpp>

#include <vector>
#include <string>
#include <map>


namespace WireCell {

    std::pair<std::string,std::string> parse_pair(const std::string& in, const std::string& delim=":");

}

#endif
