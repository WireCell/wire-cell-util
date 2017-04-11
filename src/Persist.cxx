#include "WireCellUtil/Persist.h"
#include "WireCellUtil/String.h"

// optional feature
#ifdef HAVE_LIBJSONNET_H
#include "libjsonnet++.h"
#include <cstdlib>              // for getenv
#endif


#include <boost/iostreams/copy.hpp> 
#include <boost/iostreams/filter/bzip2.hpp> 
#include <boost/iostreams/device/file.hpp> 
#include <boost/iostreams/filtering_stream.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream> 

using namespace std;
using namespace WireCell;

void WireCell::Persist::dump(const std::string& filename, const Json::Value& jroot, bool pretty)
{
    string ext = filename.substr(filename.rfind("."));

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

std::string WireCell::Persist::evaluate_jsonnet(const std::string& text,
                                                const std::string& filename)
{
#ifdef HAVE_LIBJSONNET_H
    std::string fname = "<stdin>";
    if (!filename.empty()) {
        fname = filename;
    }

    jsonnet::Jsonnet parser;
    parser.init();

    const char* cpath = std::getenv("JSONNET_PATH");
    if (cpath) {
        auto paths = String::split(cpath);
        for (int ind=0; ind<paths.size(); ++ind) {
            auto path = paths[ind];
            //cerr << "Adding Jsonnet path: " << path << " " << ind << "/" << paths.size() << endl;
            parser.addImportPath(path);
        }
    }

    std::string output; // weird API
    const bool ok =  parser.evaluateSnippet(fname, text, &output);
    if (!ok) {
        cerr << parser.lastError() << endl;
        return "";
    }
    return output;
#else
    return text;
#endif
}


Json::Value WireCell::Persist::load(const std::string& filename)
{
    string ext = filename.substr(filename.rfind("."));
    
    Json::Value jroot;

    std::fstream fp(filename.c_str(), std::ios::binary|std::ios::in);
    if (!fp) {
        cerr << "no such file: " << filename << endl;
        return jroot;
    }
    boost::iostreams::filtering_stream<boost::iostreams::input> infilt;	
    if (ext == ".bz2" ) {
	cerr << "loading compressed json file: " << filename <<"\n";
	infilt.push(boost::iostreams::bzip2_decompressor());
    }
    infilt.push(fp);
    std::string text;
    infilt >> text;
    const bool isjsonnet = filename.rfind(".jsonnet") != std::string::npos;
    return loads(text, isjsonnet);
}

Json::Value  WireCell::Persist::loads(const std::string& text, bool isjsonnet)
{
    if (isjsonnet) {
        const std::string jtext = evaluate_jsonnet(text);
        Json::Value res;
        stringstream ss(jtext);
        ss >> res;
        return res;
    }

    // duplicate this block from above to avoid yet another copy
    Json::Value res;
    stringstream ss(text);
    ss >> res;
    return res;

}

