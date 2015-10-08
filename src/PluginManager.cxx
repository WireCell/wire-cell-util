#include "WireCellUtil/PluginManager.h"

#include <string>

using namespace WireCell;
using namespace std;

WireCell::Plugin* WireCell::PluginManager::add(const char* plugin_name, const char* libname)
{
    Plugin* plugin = get(plugin_name);
    if (plugin) { return plugin; }

    string lname = libname;
    if (!libname) {
	lname = "lib";
	lname += plugin_name;
	lname += ".so";		// suck it, Mac.  ... for now
    }
    void* lib = dlopen(lname.c_str(), RTLD_NOW);
    if (!lib) { return nullptr; }
    m_plugins[plugin_name] = new Plugin(lib);
    return m_plugins[plugin_name];
}

WireCell::Plugin* WireCell::PluginManager::get(const char* plugin_name)
{
    auto pit = m_plugins.find(plugin_name);
    if (pit == m_plugins.end()) {
	return 0;
    }
    return pit->second;
}

WireCell::Plugin* WireCell::PluginManager::find(const char* symbol_name)
{
    for (auto pit : m_plugins) {
	Plugin* maybe = pit.second;
	if (maybe->contains(symbol_name)) {
	    return maybe;
	}
    }
    return nullptr;
}
