#include "WireCellUtil/PluginManager.h"

#include <string>
#include <iostream>
#include <dlfcn.h>

using namespace WireCell;
using namespace std;

Plugin::Plugin(void* lib) : m_lib(lib) {}
Plugin::~Plugin() { dlclose(m_lib); }

void* Plugin::raw(const char* symbol_name)
{
    void* ret= dlsym(m_lib, symbol_name);
    cerr << "Symbol name \"" << symbol_name << "\" @ " << ret << endl;
    return ret;
}
	
bool Plugin::contains(const char* symbol_name) 
{
    return nullptr != raw(symbol_name);
}



PluginManager& WireCell::PluginManager::instance()
{
    static PluginManager inst;
    return inst;
}

WireCell::Plugin* WireCell::PluginManager::add(const char* plugin_name, const char* libname)
{
    Plugin* plugin = get(plugin_name);
    if (plugin) {
	cerr << this << " PluginManager: already have plugin " << plugin_name << endl;
	return plugin;
    }

    string lname = "";
    if (libname) {
	lname = libname;
    }
    else {
	lname = "lib";
	lname += plugin_name;
	lname += ".so";		// suck it, Mac.  ... for now
    }
    void* lib = dlopen(lname.c_str(), RTLD_NOW);
    if (!lib) {
	cerr << this << " PluginManager: failed to open plugin library " << lname << endl;
	return nullptr;
    }
    m_plugins[plugin_name] = new Plugin(lib);
    cerr << this << " PluginManager: loaded plugin #" << m_plugins.size() << " \"" << plugin_name << "\" from library \"" << lname << "\": " << m_plugins[plugin_name] << endl;
    return m_plugins[plugin_name];
}

WireCell::Plugin* WireCell::PluginManager::get(const char* plugin_name)
{
    auto pit = m_plugins.find(plugin_name);
    if (pit == m_plugins.end()) {
	return nullptr;
    }
    return pit->second;
}

WireCell::Plugin* WireCell::PluginManager::find(const char* symbol_name)
{
    for (auto pit : m_plugins) {
	Plugin* maybe = pit.second;
	if (maybe->contains(symbol_name)) {
	    cerr << this << " PluginManager: found symbol \"" << symbol_name << "\" in plugin: \"" << pit.first << "\"" << endl;
	    return maybe;
	}
	cerr << this << " PluginManager: no symbol \"" << symbol_name << "\" in plugin: \"" << pit.first << "\"" << endl;
    }
    cerr << this << " PluginManager: no symbol \"" << symbol_name << "\" found in " << m_plugins.size() << " plugins" << endl;
    return nullptr;
}

WireCell::PluginManager::PluginManager()
{
    cerr << this << " PluginManager starting" << endl;
}
WireCell::PluginManager::~PluginManager()
{
    cerr << this << " PluginManager terminating" << endl;
    for (auto pit : m_plugins) {
	delete pit.second;
	pit.second = nullptr;
    }
}
