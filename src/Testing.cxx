#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Exceptions.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace WireCell;

void boost::assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line)
{
    stringstream ss;
    ss << "WireCell::AssertionError: \"" << expr << "\" in " << function << " " << file << ":" << line;
    if (msg and msg[0]) {
        ss << "\n" << msg;
    }
    cerr << ss.str() << endl;
    THROW(AssertionError() << errmsg{ss.str()});
}

void boost::assertion_failed(char const * expr, char const * function, char const * file, long line)
{
    boost::assertion_failed_msg(expr, "", function, file, line);
}

