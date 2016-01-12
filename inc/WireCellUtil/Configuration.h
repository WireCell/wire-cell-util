#ifndef WIRECELL_CONFIGURATION
#define WIRECELL_CONFIGURATION

#include <json/json.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sstream>

namespace WireCell {

    /** The data type for configuration.
     *
     * Configuration is a recursive in that one Configuration object
     * may contain others.
     *
     * The Configuration object given to a component must be the top
     * of a component-specific schema.
     *
     * Configuration objects are associated to a component using the
     * component's class name and an optional instance name.
     * Configurations of multiple components may be aggregated through
     * a two level dictionary.  Top level keys are component class
     * names.  Keys of these dictionaries are instance names.  The
     * empty string is interpreted as an unspecified (or default)
     * instance.
     */

    typedef Json::Value Configuration;

    /// Load a configuration from the named file.  If format it given,
    /// force that format, otherwise divine it from the filename
    /// extension.
    Configuration configuration_load(const std::string& filename,
				     const std::string& format="json");

    /// Load a configuration from a string assuming the given format.
    Configuration configuration_loads(const std::string& data,
				      const std::string& format="json");

    /// Dump a configuration to the named file
    bool configuration_dump(const std::string& filename, const Configuration& cfg,
			    const std::string& format="json");

    /// Dump a configuration to a string
    std::string configuration_dumps(const Configuration& cfg,
				    const std::string& format="json");

    
    /// Convert a configuration value to a particular type.
    template<typename T>
    T convert(const Configuration& cfg, const T& def = T()) {
	return def;
    }
    template<>
    int convert<int>(const Configuration& cfg, const int& def) {
	return cfg.asInt();
    }
    template<>
    float convert<float>(const Configuration& cfg, const float& def) {
	return cfg.asInt();
    }
    template<>
    double convert<double>(const Configuration& cfg, const double& def) {
	return cfg.asDouble();
    }
    template<> 
    std::string convert<std::string>(const Configuration& cfg, const std::string& def) {
	return cfg.asString();
    }
    template<>
    std::vector<std::string> convert< std::vector<std::string> >(const Configuration& cfg, const std::vector<std::string>& def) {
	std::vector<std::string> ret;
	for (auto v : cfg) {
	    ret.push_back(convert<std::string>(v));
	}
	return ret;
    }

    /// Follow a dot.separated.path and return the branch there.
    Configuration branch(Configuration cfg, const std::string& dotpath);

    /// Merge b into a, return a
    Configuration update(Configuration& a, Configuration& b);

    /// Get value in configuration at the dotted path from or return default.
    template<typename T>
    T get(Configuration cfg, const std::string& dotpath, const T& def = T()) {
	return convert(branch(cfg, dotpath), def);
    }

    /// Put value in configuration at the dotted path.
    template<typename T>
    void put(Configuration& cfg, const std::string& dotpath, const T& val) {
	Configuration* ptr = &cfg;
	std::vector<std::string> path;
	boost::algorithm::split(path, dotpath, boost::algorithm::is_any_of("."));
	for (auto name : path) {
	    ptr = &(*ptr)[name];
	}
	*ptr = val;
    }



} // namespace WireCell

#endif
