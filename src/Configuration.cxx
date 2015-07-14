#include "WireCellUtil/Configuration.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <sstream>

using namespace WireCell;
using namespace std;

/// Load a configuration from the named file.
Configuration  WireCell::configuration_load(const std::string& filename,
					    const std::string& format)
{
    Configuration res;
    if (format == "xml" || boost::algorithm::ends_with(filename, ".xml")) {
	boost::property_tree::read_xml(filename, res);
    }
    if (format == "json" || boost::algorithm::ends_with(filename, ".json")) {
	boost::property_tree::read_json(filename, res);
    }
    if (format == "ini" || boost::algorithm::ends_with(filename, ".ini")) {
	boost::property_tree::read_ini(filename, res);
    }
    if (format == "info" || boost::algorithm::ends_with(filename, ".info")) {
	boost::property_tree::read_info(filename, res);
    }
    return res;
}


/// Load a configuration from a string
Configuration  WireCell::configuration_loads(const std::string& data,
					     const std::string& format)
{
    stringstream ss;
    ss << data;
    Configuration res;
    if (format == "xml") {
	boost::property_tree::read_xml(ss, res);
    }
    if (format == "json") {
	boost::property_tree::read_json(ss, res);
    }
    if (format == "ini") {
	boost::property_tree::read_ini(ss, res);
    }
    if (format == "info") {
	boost::property_tree::read_info(ss, res);
    }
    return res;
}

    /// Dump a configuration to the named file
bool  WireCell::configuration_dump(const std::string& filename, const Configuration& cfg,
				   const std::string& format)
{
    if (format == "xml" || boost::algorithm::ends_with(filename, ".xml")) {
	boost::property_tree::write_xml(filename, cfg);
    }
    if (format == "json" || boost::algorithm::ends_with(filename, ".json")) {
	boost::property_tree::write_json(filename, cfg);
    }
    if (format == "ini" || boost::algorithm::ends_with(filename, ".ini")) {
	boost::property_tree::write_ini(filename, cfg);
    }
    if (format == "info" || boost::algorithm::ends_with(filename, ".info")) {
	boost::property_tree::write_info(filename, cfg);
    }
    return true;		// fixme: do error checking
}

/// Dump a configuration to a string.
std::string  WireCell::configuration_dumps(const Configuration& cfg,
					   const std::string& format)
{
    stringstream ss;
    if (format == "xml") {
	boost::property_tree::write_xml(ss, cfg);
    }
    if (format == "json") {
	boost::property_tree::write_json(ss, cfg);
    }
    if (format == "ini") {
	boost::property_tree::write_ini(ss, cfg);
    }
    if (format == "info") {
	boost::property_tree::write_info(ss, cfg);
    }
    return ss.str();
}

