#include "WireCellUtil/String.h"

using namespace std;

std::pair<std::string,std::string> WireCell::parse_pair(const std::string& in, const std::string& delim)
{
    vector<string> chunks;
    boost::split(chunks, in, boost::is_any_of(delim), boost::token_compress_on);
    
    string first = chunks[0];
    string second = "";
    if (chunks.size() > 1) {
	second = chunks[1];
    }
    return make_pair(first, second);
}

