#include "WireCellUtil/Configuration.h"

#include <sstream>
#include <fstream>

using namespace WireCell;
using namespace std;

/// Load a configuration from the named file.
Configuration  WireCell::configuration_load(const std::string& filename,
					    const std::string& format)
{
    Configuration res;
    if (format != "json") {
	return res;
    }
    ifstream fstr(filename);
    fstr >> res;
    return res;
}


/// Load a configuration from a string
Configuration  WireCell::configuration_loads(const std::string& data,
					     const std::string& format)
{
    Configuration res;
    if (format != "json") {
	return res;
    }
    stringstream ss(data);
    ss >> res;
    return res;
}

    /// Dump a configuration to the named file
bool  WireCell::configuration_dump(const std::string& filename,
				   const Configuration& cfg,
				   const std::string& format)
{
    if (format != "json") {
	return false;
    }
    ofstream fstr(filename);
    fstr << cfg;
    return true;		// fixme: do error checking
}

/// Dump a configuration to a string.
std::string  WireCell::configuration_dumps(const Configuration& cfg,
					   const std::string& format)
{
    if (format != "json") {
	return "";
    }
    stringstream ss;
    ss << cfg;
    return ss.str();
}
