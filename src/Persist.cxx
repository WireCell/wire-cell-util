#include "WireCellUtil/Persist.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/Logging.h"
#include "WireCellUtil/Exceptions.h"

#include "libjsonnet++.h"

#include <cstdlib>              // for getenv, see get_path()


#include <boost/iostreams/copy.hpp> 
#include <boost/iostreams/filter/bzip2.hpp> 
#include <boost/iostreams/device/file.hpp> 
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <sstream>
#include <fstream>

using spdlog::info;
using spdlog::error;
using namespace std;
using namespace WireCell;

#define WIRECELL_PATH_VARNAME "WIRECELL_PATH"

static std::string file_extension(const std::string& filename)
{
    auto ind = filename.rfind(".");
    if (ind == string::npos) {
        return "";
    }
    return filename.substr(ind);
}

void WireCell::Persist::dump(const std::string& filename, const Json::Value& jroot, bool pretty)
{
    string ext = file_extension(filename);

    /// default to .json.bz2 regardless of extension.
    std::fstream fp(filename.c_str(), std::ios::binary|std::ios::out);
    boost::iostreams::filtering_stream<boost::iostreams::output> outfilt;
    if (ext == ".bz2") {
	outfilt.push(boost::iostreams::bzip2_compressor());
    }
    outfilt.push(fp);
    if (pretty) {
	Json::StyledWriter jwriter;
	outfilt << jwriter.write(jroot);
    }
    else {
	Json::FastWriter jwriter;
	outfilt << jwriter.write(jroot);
    }
}
// fixme: support pretty option for indentation
std::string  WireCell::Persist::dumps(const Json::Value& cfg, bool)
{
    stringstream ss;
    ss << cfg;
    return ss.str();
}

std::string WireCell::Persist::slurp(const std::string& filename)
{
    std::string fname = resolve(filename);
    if (fname.empty()) {
        THROW(IOError() << errmsg{"no such file: " + filename + ". Maybe you need to add to WIRECELL_PATH."});
    }

    std::ifstream fstr(filename);
    std::stringstream buf;
    buf << fstr.rdbuf();
    return buf.str();
}


bool WireCell::Persist::exists(const std::string& filename)
{
    return boost::filesystem::exists(filename);
}

static std::vector<std::string> get_path()
{
    std::vector<std::string> ret;
    const char* cpath = std::getenv(WIRECELL_PATH_VARNAME);
    if (!cpath) {
        return ret;
    }
    for (auto path : String::split(cpath)) {
        ret.push_back(path);
    }
    return ret;
}



std::string WireCell::Persist::resolve(const std::string& filename)
{
    if (filename.empty()) {
        return "";
    }
    if (filename[0] == '/') {
        return filename;
    }

    std::vector<boost::filesystem::path> tocheck{boost::filesystem::current_path(),};
    for (auto pathname : get_path()) {
        tocheck.push_back(boost::filesystem::path(pathname));
    }
    for (auto pobj : tocheck) {
        boost::filesystem::path full = pobj / filename;
        if (boost::filesystem::exists(full)) {
            return boost::filesystem::canonical(full).string();
        }
    }
    return "";
}

Json::Value WireCell::Persist::load(const std::string& filename,
                                    const externalvars_t& extvar,
                                    const externalvars_t& extcode)
{
    string ext = file_extension(filename);
    if (ext == ".jsonnet") {    // use libjsonnet++ file interface
        string text = evaluate_jsonnet_file(filename, extvar, extcode);
        return json2object(text);
    }

    std::string fname = resolve(filename);
    if (fname.empty()) {
        THROW(IOError() << errmsg{"no such file: " + filename + ". Maybe you need to add to WIRECELL_PATH."});
    }


    // use jsoncpp file interface
    std::fstream fp(fname.c_str(), std::ios::binary|std::ios::in);
    boost::iostreams::filtering_stream<boost::iostreams::input> infilt;	
    if (ext == ".bz2" ) {
        info("loading compressed json file: {}", fname);
	infilt.push(boost::iostreams::bzip2_decompressor());
    }
    infilt.push(fp);
    std::string text;
    Json::Value jroot;    
    infilt >> jroot;
    //return update(jroot, extvar); fixme
    return jroot;
}

Json::Value  WireCell::Persist::loads(const std::string& text,
                                      const externalvars_t& extvar,
                                      const externalvars_t& extcode)
{
    const std::string jtext = evaluate_jsonnet_text(text, extvar, extcode);
    return json2object(jtext);
}

// bundles few lines into function to avoid some copy-paste
Json::Value WireCell::Persist::json2object(const std::string& text)
{
    Json::Value res;
    stringstream ss(text);
    ss >> res;
    return res;
}


static void init_parser(jsonnet::Jsonnet& parser,
                        const Persist::externalvars_t& extvar,
                        const Persist::externalvars_t& extcode)
{
    parser.init();
    for (auto path : get_path()) {
        parser.addImportPath(path);
    }
    for (auto& vv : extvar) {
        parser.bindExtVar(vv.first, vv.second);
    }
    for (auto& vv : extcode) {
        parser.bindExtCodeVar(vv.first, vv.second);
    }
}
std::string WireCell::Persist::evaluate_jsonnet_file(const std::string& filename,
                                                     const externalvars_t& extvar,
                                                     const externalvars_t& extcode)
{
    std::string fname = resolve(filename);
    if (fname.empty()) {
        THROW(IOError() << errmsg{"no such file: " + filename + ", maybe you need to add to WIRECELL_PATH."});
    }

    jsonnet::Jsonnet parser;
    init_parser(parser, extvar, extcode);

    std::string output;
    const bool ok = parser.evaluateFile(fname, &output);
    if (!ok) {
        error(parser.lastError());
        THROW(ValueError() << errmsg{parser.lastError()});
    }
    return output;
}
std::string WireCell::Persist::evaluate_jsonnet_text(const std::string& text,
                                                     const externalvars_t& extvar,
                                                     const externalvars_t& extcode)
{
    jsonnet::Jsonnet parser;
    init_parser(parser, extvar, extcode);

    std::string output;
    bool ok =  parser.evaluateSnippet("<stdin>", text, &output);
    if (!ok) {
        error(parser.lastError());
        THROW(ValueError() << errmsg{parser.lastError()});
    }
    return output;
}

WireCell::Persist::Parser::Parser(const std::vector<std::string>& load_paths,
                                  const externalvars_t& extvar,
                                  const externalvars_t& extcode)
{
    m_jsonnet.init();

    // Loading: 1) cwd, 2) passed in paths 3) environment
    m_load_paths.push_back(boost::filesystem::current_path());
    for (auto path : load_paths) {
        m_load_paths.push_back(boost::filesystem::path(path));
    }
    for (auto path : get_path()) {
        m_load_paths.push_back(boost::filesystem::path(path));
    }
    // load paths into jsonnet backwards to counteract its reverse ordering
    for (auto pit = m_load_paths.rbegin(); pit != m_load_paths.rend(); ++pit) {
        m_jsonnet.addImportPath(boost::filesystem::canonical(*pit).string());
    }


    // external variables
    for (auto& vv : extvar) {
        m_jsonnet.bindExtVar(vv.first, vv.second);
    }
    // external code
    for (auto& vv : extcode) {
        m_jsonnet.bindExtCodeVar(vv.first, vv.second);
    }
}
            
            


///
/// Below is a reimplimenatiaon of the above but in class form.....
////

std::string WireCell::Persist::Parser::resolve(const std::string& filename)
{
    if (filename.empty()) {
        return "";
    }
    if (filename[0] == '/') {
        return filename;
    }

    for (auto pobj : m_load_paths) {
        boost::filesystem::path full = pobj / filename;
        if (boost::filesystem::exists(full)) {
            return boost::filesystem::canonical(full).string();
        }
    }
    return "";
}

Json::Value WireCell::Persist::Parser::load(const std::string& filename)
{
    std::string fname = resolve(filename);
    if (fname.empty()) {
        THROW(IOError() << errmsg{"no such file: " + filename
                    + ". Maybe you need to add to WIRECELL_PATH."});
    }
    string ext = file_extension(filename);

    if (ext == ".jsonnet" or ext.empty()) {    // use libjsonnet++ file interface
        std::string output;
        const bool ok = m_jsonnet.evaluateFile(fname, &output);
        if (!ok) {
            error(m_jsonnet.lastError());
            THROW(ValueError() << errmsg{m_jsonnet.lastError()});
        }
        return json2object(output);
    }

    // also support JSON, possibly compressed

    // use jsoncpp file interface
    std::fstream fp(fname.c_str(), std::ios::binary|std::ios::in);
    boost::iostreams::filtering_stream<boost::iostreams::input> infilt;	
    if (ext == ".bz2" ) {
	info("loading compressed json file: {}", fname);
	infilt.push(boost::iostreams::bzip2_decompressor());
    }
    infilt.push(fp);
    std::string text;
    Json::Value jroot;    
    infilt >> jroot;
    //return update(jroot, extvar); fixme
    return jroot;
}

Json::Value WireCell::Persist::Parser::loads(const std::string& text)
{
    std::string output;
    bool ok =  m_jsonnet.evaluateSnippet("<stdin>", text, &output);
    if (!ok) {
        error(m_jsonnet.lastError());
        THROW(ValueError() << errmsg{m_jsonnet.lastError()});
    }
    return json2object(output);
}
