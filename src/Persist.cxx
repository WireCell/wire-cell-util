#include "WireCellUtil/Persist.h"


#include <boost/iostreams/copy.hpp> 
#include <boost/iostreams/filter/bzip2.hpp> 
#include <boost/iostreams/device/file.hpp> 
#include <boost/iostreams/filtering_stream.hpp>

#include <string>
#include <fstream>
#include <iostream> 

using namespace std;

void WireCell::Persist::dump(const char* filename, Json::Value& jroot, bool pretty)
{
    string fname=filename;
    string ext = fname.substr(fname.rfind("."));

    /// default to .json.bz2 regardless of extension.
    std::fstream fp(filename, std::ios::binary|std::ios::out);
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

Json::Value WireCell::Persist::load(const char* filename)
{
    string fname=filename;
    string ext = fname.substr(fname.rfind("."));
    
    Json::Value jroot;
    std::fstream fp(filename, std::ios::binary|std::ios::in);
    boost::iostreams::filtering_stream<boost::iostreams::input> infilt;	
    if (ext == ".bz2" ) {
	cerr << "loading compressed json file\n";
	infilt.push(boost::iostreams::bzip2_decompressor());
    }
    infilt.push(fp);
    infilt >> jroot;
    return jroot;
}

