#ifndef WIRECELL_PLUGINMANAGER
#define WIRECELL_PLUGINMANAGER

#include <dlfcn.h>
#include <map>

namespace WireCell {

    class Plugin {
    public:
	Plugin(void* lib) : m_lib(lib) {}
	~Plugin() { dlclose(m_lib); }

	void* raw(const char* symbol_name) {
	    return dlsym(m_lib, symbol_name);
	}
	
	bool contains(const char* symbol_name) {
	    return nullptr != raw(symbol_name);
	}

	template<typename T>
	bool symbol(const char* symbol_name, T& ret) {
	    void* thing = raw(symbol_name);
	    if (!thing) { return false; } 
	    ret = reinterpret_cast<T>(thing);
	    return true;
	}
    private:
	void* m_lib;
    };

    /** This is meant to be used from a WireCell::Singleton. */
    class PluginManager{
    public:

	/// Add a plugin.  If libname is not given, try to derive it.
	Plugin* add(const char* plugin_name, const char* libname = nullptr);

	Plugin* get(const char* plugin_name);

	Plugin* find(const char* symbol_name);
    private:
	std::map<std::string, Plugin*> m_plugins;
    };

}

#endif

