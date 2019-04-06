#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Logging.h"

#include <random>
#include <iostream>
#include <functional>

using spdlog::info;

using namespace std;
using namespace WireCell;

int main() {

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    auto uni = std::bind(distribution, generator);

    info("Generating data");
    const int nbins = 5000;
    const int narrays = 100;
    Json::Value jroot;
    for (int ind=0; ind<narrays; ++ind) {
	Json::Value jarr;
	for (int ibin=0; ibin<nbins; ++ibin) {
	    auto number = uni();
	    jarr.append(number);
	}
	jroot[ind] = jarr;
    }

    // dump non-pretty in plain text and bzip2'ed to see size diff
    info("Dump test_persist-ugly.json");
    Persist::dump("test_persist-ugly.json", jroot);
    info("Dump test_persist-ugly.json.bz2");
    Persist::dump("test_persist-ugly.json.bz2", jroot);

    // dump pretty, read back and redump so can do a diff
    info("Dump test_persist-pretty.json");
    Persist::dump("test_persist-pretty.json", jroot, true);
    info("Dump test_persist-pretty.json.bz2");
    Persist::dump("test_persist-pretty.json.bz2", jroot, true);

    info("Load test_persist-pretty.json");
    Json::Value jroot2 = Persist::load("test_persist-pretty.json");
    info("Load test_persist-pretty.json.bz2");
    Json::Value jroot3 = Persist::load("test_persist-pretty.json.bz2");

    info("diff test_persist-pretty.json test_persist-pretty2.json");
    Persist::dump("test_persist-pretty2.json", jroot2, true);
    info("diff test_persist-pretty.json test_persist-pretty2.json");
    Persist::dump("test_persist-pretty3.json", jroot3, true);
    return 0;

}
