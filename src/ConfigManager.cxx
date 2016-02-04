#include "WireCellUtil/ConfigManager.h"
#include "WireCellUtil/NamedFactory.h"

#include <iostream>

using namespace std;
using namespace WireCell;

ConfigManager::ConfigManager()
    : m_top(configuration_loads("[]"))
{
}
ConfigManager::~ConfigManager()
{
}

void ConfigManager::load(const std::string& filename)
{
    Configuration more = configuration_load(filename);
    m_top = append(m_top, more);
}

void ConfigManager::loads(const std::string& jsonstring)
{
    Configuration more = configuration_loads(jsonstring);
    m_top = append(m_top, more);
}

void ConfigManager::dump(const std::string& filename) const
{
    configuration_dump(filename, m_top);
}

std::string ConfigManager::dumps() const
{
    return configuration_dumps(m_top);
}


int ConfigManager::index(const std::string& type, const std::string& name) const
{
    int ind = -1;
    for (auto c : m_top) {
	++ind;
	if (get<string>(c, "type") != type) {
	    continue;
	}
	if (get<string>(c, "name") != name) {
	    continue;
	}
	return ind;
    }
    return -1;

}

int ConfigManager::add(Configuration& cfg)
{
    int ind = this->index(get<string>(cfg, "type"), get<string>(cfg, "name"));
    if (ind < 0) {
	ind = m_top.size();
    }
    m_top[ind] = cfg;
    return ind;
}

int ConfigManager::add(Configuration& payload, const std::string& type, const std::string& name)
{
    Configuration cfg;
    cfg["data"] = payload;
    cfg["type"] = type;
    cfg["name"] = name;
    return add(cfg);
}

Configuration ConfigManager::at(int ind) const
{
    if (ind < 0 || ind >= size()) {
	return Configuration();
    }
    return m_top[ind];
}

std::vector<ConfigManager::ClassInstance> ConfigManager::configurables() const
{
    std::vector<ConfigManager::ClassInstance> ret;
    for (auto c : m_top) {
	ret.push_back(make_pair(get<string>(c, "type"), get<string>(c, "name")));
    }
    return ret;
}

Configuration ConfigManager::pop(int ind)
{
    if (ind < 0 || ind >= size()) {
	return Configuration();
    }
    Configuration ret;
    Configuration reduced = configuration_loads("[]");
    int siz = size();
    for (int i=0; i<siz; ++i) {
	if (i == ind) {
	    ret = m_top[ind];
	}
	else {
	    reduced.append(m_top[ind]);
	}
    }
    m_top = reduced;
    return ret;
}
