#include "WireCellUtil/Persist.h"
#include "WireCellUtil/String.h"
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
#include <iostream> 

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
                                    const externalvars_t& extvar)
{
    string ext = file_extension(filename);
    if (ext == ".jsonnet") {    // use libjsonnet++ file interface
        string text = evaluate_jsonnet_file(filename, extvar);
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
	cerr << "WCT: loading compressed json file: " << fname <<"\n";
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
                                      const externalvars_t& extvar)
{
    const std::string jtext = evaluate_jsonnet_text(text, extvar);
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


static void init_parser(jsonnet::Jsonnet& parser, const Persist::externalvars_t& extvar)
{
    parser.init();
    for (auto path : get_path()) {
        parser.addImportPath(path);
    }
    for (auto& vv : extvar) {
        //cerr << "extra var: \"" << vv.first << "\" = \"" << vv.second << "\"\n";
        parser.bindExtCodeVar(vv.first, vv.second);
    }
}
std::string WireCell::Persist::evaluate_jsonnet_file(const std::string& filename,
                                                     const externalvars_t& extvar)
{
    std::string fname = resolve(filename);
    if (fname.empty()) {
        THROW(IOError() << errmsg{"no such file: " + filename + ", maybe you need to add to WIRECELL_PATH."});
    }

    jsonnet::Jsonnet parser;
    init_parser(parser, extvar);

    std::string output; // weird API
    const bool ok = parser.evaluateFile(fname, &output);
    if (!ok) {
        cerr << parser.lastError() << endl;
        THROW(ValueError() << errmsg{parser.lastError()});
    }
    return output;
}
std::string WireCell::Persist::evaluate_jsonnet_text(const std::string& text,
                                                     const externalvars_t& extvar)
{
    jsonnet::Jsonnet parser;
    init_parser(parser, extvar);

    std::string output; // weird API
    bool ok =  parser.evaluateSnippet("<stdin>", text, &output);
    if (!ok) {
        cerr << parser.lastError() << endl;
        THROW(ValueError() << errmsg{parser.lastError()});
    }
    return output;
}
