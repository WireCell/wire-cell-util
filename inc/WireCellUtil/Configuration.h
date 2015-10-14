#ifndef WIRECELL_CONFIGURATION
#define WIRECELL_CONFIGURATION

#include <json/json.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sstream>

namespace WireCell {

    /// Use Boost property trees as the WireCell::Configuration store.
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

    template<typename T>
    T get(Configuration cfg, const std::string& dotpath, const T& def = T()) {
	std::vector<std::string> path;
	boost::algorithm::split(path, dotpath, boost::algorithm::is_any_of("."));
	for (auto name : path) {
	    cfg = cfg[name];
	}
	return convert(cfg, def);
    }

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

    /** Recursively traverse a configuration starting at a child node.
     *
     * See also WireCell::configuration_visit()
     *
     * \param cfg is the configuration.
     * \param childPath is the path to the child.
     * \param child is the child.
     * \param method is a callable taking (cfg,childPath,child).
     *
     * Adapted from:
     * http://stackoverflow.com/a/8175833
     */
    // template<typename Callable>
    // void configuration_visit_recursive(const Configuration &cfg,
    // 				       const Configuration::path_type &childPath,
    // 				       const Configuration &child,
    // 				       Callable &method)
    // {
    // 	method(cfg, childPath, child);
    // 	for (Configuration::const_iterator it = child.begin(); it != child.end(); ++it) {
    // 	    Configuration::path_type curPath = childPath / Configuration::path_type(it->first);
    // 	    configuration_visit_recursive(cfg, curPath, it->second, method);
    // 	}
    // }

    /** Visit all nodes in a configuration tree.
     *
     * \param cfg is the configuration.
     * \param method is a callable taking (cfg,childPath,child).
     */
    // template<typename Callable>
    // void configuration_visit(const Configuration &cfg, Callable &method)
    // {
    // 	configuration_visit_recursive(cfg, "", cfg, method);
    // }
    
    /** Helper to merge one configuration into another.
     *
     *     ConfigurationMerge merger(mycfg);
     *     merger(some_config);
     *     merger(some_other_config);
     *     use_config(mycfg);
     *
     * Warning: all child paths must be unique.
     */
    // struct ConfigurationMerge {
    // 	Configuration& cfg;
    // 	ConfigurationMerge(Configuration& config) : cfg(config) {};

    // 	Configuration& operator()(const Configuration &other) {
    // 	    configuration_visit(other, *this);
    // 	}

    // 	void operator()(const Configuration &other,
    // 			const Configuration::path_type &childPath,
    // 			const Configuration &child) {
    // 	    cfg.put(childPath, child.data());
    // 	}
    // };


} // namespace WireCell

#endif
