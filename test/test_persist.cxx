#include "WireCellUtil/Persist.h"

#include <random>
#include <iostream>
#include <functional>


using namespace std;
using namespace WireCell;

int main() {

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    auto uni = std::bind(distribution, generator);

    cout << "Generating data\n";
    const int nbins = 5000;
    const int narrays = 100;
    double data[narrays][nbins];
    Json::Value jroot;
    for (int ind=0; ind<narrays; ++ind) {
	Json::Value jarr;
	for (int ibin=0; ibin<nbins; ++ibin) {
	    auto number = uni();
	    data[ind][ibin] = number;
	    jarr.append(number);
	}
	jroot[ind] = jarr;
    }

    // dump non-pretty in plain text and bzip2'ed to see size diff
    cout << "Dump test_persist-ugly.json\n";
    Persist::dump("test_persist-ugly.json", jroot);
    cout << "Dump test_persist-ugly.json.bz2\n";
    Persist::dump("test_persist-ugly.json.bz2", jroot);

    // dump pretty, read back and redump so can do a diff
    cout << "Dump test_persist-pretty.json\n";
    Persist::dump("test_persist-pretty.json", jroot, true);
    cout << "Dump test_persist-pretty.json.bz2\n";
    Persist::dump("test_persist-pretty.json.bz2", jroot, true);

    cout << "Load test_persist-pretty.json\n";
    Json::Value jroot2 = Persist::load("test_persist-pretty.json");
    cout << "Load test_persist-pretty.json.bz2\n";
    Json::Value jroot3 = Persist::load("test_persist-pretty.json.bz2");

    cout << "diff test_persist-pretty.json test_persist-pretty2.json\n";
    Persist::dump("test_persist-pretty2.json", jroot2, true);
    cout << "diff test_persist-pretty.json test_persist-pretty2.json\n";
    Persist::dump("test_persist-pretty3.json", jroot3, true);
    return 0;

}
