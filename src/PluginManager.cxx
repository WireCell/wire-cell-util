#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/Exceptions.h"

#include <string>
#include <iostream>
#include <dlfcn.h>

using namespace WireCell;
using namespace std;

Plugin::Plugin(void* lib) : m_lib(lib) {}
Plugin::~Plugin() { dlclose(m_lib); }

void* Plugin::raw(const std::string& symbol_name)
{
    void* ret= dlsym(m_lib, symbol_name.c_str());
    //cerr << "Symbol name \"" << symbol_name << "\" @ " << ret << endl;
    return ret;
}
	
bool Plugin::contains(const std::string& symbol_name) 
{
    return nullptr != raw(symbol_name);
}



PluginManager& WireCell::PluginManager::instance()
{
    static PluginManager inst;
    return inst;
}

WireCell::Plugin* WireCell::PluginManager::add(const std::string& plugin_name,
					       const std::string& libname)
{
    Plugin* plugin = get(plugin_name);
    if (plugin) {
	cerr << " PluginManager: already have plugin " << plugin_name << endl;
	return plugin;
    }

    std::string exts[2] = {".so",".dylib"};
    for (int ind=0; ind<2; ++ind) {
        std::string ext = exts[ind];
        string lname = "";
        if (libname == "") {
            lname = "lib";
            lname += plugin_name;
            lname += ext;
        }
        else {
            lname = libname;
        }
        void* lib = dlopen(lname.c_str(), RTLD_NOW);
        if (lib) {
            m_plugins[plugin_name] = new Plugin(lib);
            // cerr << " PluginManager: loaded plugin #" << m_plugins.size() << " \"" << plugin_name << "\" from library \"" << lname << "\": " << m_plugins[plugin_name] << endl;
            return m_plugins[plugin_name];
        }
    }
    cerr << "PluginManager: no such plugin: " << plugin_name << "\n";
    THROW(IOError() << errmsg{"no such plugin: " + plugin_name});
    return nullptr;
}

WireCell::Plugin* WireCell::PluginManager::get(const std::string& plugin_name)
{
    auto pit = m_plugins.find(plugin_name);
    if (pit == m_plugins.end()) {
	return nullptr;
    }
    return pit->second;
}

WireCell::Plugin* WireCell::PluginManager::find(const std::string& symbol_name)
{
    for (auto pit : m_plugins) {
	Plugin* maybe = pit.second;
	if (maybe->contains(symbol_name)) {
	    //cerr << this << " PluginManager: found symbol \"" << symbol_name << "\" in plugin: \"" << pit.first << "\"" << endl;
	    return maybe;
	}
	// cerr << this << " PluginManager: no symbol \"" << symbol_name << "\" in plugin: \"" << pit.first << "\"" << endl;
    }
    // cerr << this << " PluginManager: no symbol \"" << symbol_name << "\" found in " << m_plugins.size() << " plugins" << endl;
    return nullptr;
}

WireCell::PluginManager::PluginManager()
{
    //cerr << this << " PluginManager starting" << endl;
}
WireCell::PluginManager::~PluginManager()
{
    //cerr << this << " PluginManager terminating" << endl;
    for (auto pit : m_plugins) {
	delete pit.second;
	pit.second = nullptr;
    }
}
