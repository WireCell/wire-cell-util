#ifndef WIRECELL_CONFIGURATION
#define WIRECELL_CONFIGURATION

#include <boost/property_tree/ptree.hpp>

namespace WireCell {

    /// Use Boost property trees as the WireCell::Configuration store.
    typedef boost::property_tree::ptree Configuration;

    /// Load a configuration from the named file.  If format it given,
    /// force that format, otherwise divine it from the filename
    /// extension.
    Configuration configuration_load(const std::string& filename,
				     const std::string& format="");

    /// Load a configuration from a string assuming the given format.
    Configuration configuration_loads(const std::string& data,
				      const std::string& format="json");

    /// Dump a configuration to the named file
    bool configuration_dump(const std::string& filename, const Configuration& cfg,
			    const std::string& format="");

    /// Dump a configuration to a string
    std::string configuration_dumps(const Configuration& cfg,
				    const std::string& format="json");

    
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
    template<typename Callable>
    void configuration_visit_recursive(const Configuration &cfg,
				       const Configuration::path_type &childPath,
				       const Configuration &child,
				       Callable &method)
    {
	method(cfg, childPath, child);
	for (Configuration::const_iterator it = child.begin(); it != child.end(); ++it) {
	    Configuration::path_type curPath = childPath / Configuration::path_type(it->first);
	    configuration_visit_recursive(cfg, curPath, it->second, method);
	}
    }

    /** Visit all nodes in a configuration tree.
     *
     * \param cfg is the configuration.
     * \param method is a callable taking (cfg,childPath,child).
     */
    template<typename Callable>
    void configuration_visit(const Configuration &cfg, Callable &method)
    {
	configuration_visit_recursive(cfg, "", cfg, method);
    }
    
    /** Helper to merge one configuration into another.
     *
     *     ConfigurationMerge merger(mycfg);
     *     merger(some_config);
     *     merger(some_other_config);
     *     use_config(mycfg);
     *
     * Warning: all child paths must be unique.
     */
    struct ConfigurationMerge {
	Configuration& cfg;
	ConfigurationMerge(Configuration& config) : cfg(config) {};

	Configuration& operator()(const Configuration &other) {
	    configuration_visit(other, *this);
	}

	void operator()(const Configuration &other,
			const Configuration::path_type &childPath,
			const Configuration &child) {
	    cfg.put(childPath, child.data());
	}
    };

} // namespace WireCell

#endif
