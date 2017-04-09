#include "WireCellUtil/Persist.h"

// optional feature
#ifdef HAVE_LIBJSONNET_H
#include "libjsonnet++.h"
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


Json::Value WireCell::Persist::load(const std::string& filename)
{
    string ext = filename.substr(filename.rfind("."));
    
    Json::Value jroot;

// this feature is optional contingent on compiling in support for
// jsonnet.  Compression isn't supported on jsonnet files because I'm
// lazy.  The whole point is that they are small and hand crafted.
#ifdef HAVE_LIBJSONNET_H
    if (ext == ".jsonnet") {
        // fixme: need a way to specify equivalent of the -J path that
        // jsonnet CLI accepts so that user jsonnet code can find
        // wirecell.libsonnet, etc.

        jsonnet::Jsonnet parser;
        parser.init();

        std::string output; // weird API
        const bool ok = parser.evaluateFile(filename, &output);
        if (!ok) {
            cerr << "failed to evaluate " << filename << endl;
            return jroot;
        }
        jroot = loads(output);
        return jroot;
    }
#endif

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
    infilt >> jroot;
    return jroot;
}

Json::Value  WireCell::Persist::loads(const std::string& text)
{
    Json::Value res;
    stringstream ss(text);
    ss >> res;
    return res;
}

