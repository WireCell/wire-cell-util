#include "WireCellUtil/ImpactResponse.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

using namespace WireCell;
using namespace std;


int main(int argc, const char* argv[])
{

    if (argc < 2) {
	cerr << "This test requires an Wire Cell Field Response input file." << endl;
	return 0;
    }

    WireCell::ExecMon em("test_persist_responses");
    auto fr = Response::Schema::load(argv[1]);
    em("loaded");

    PlaneImpactResponse u(fr,0), v(fr,1), w(fr,2);
    
    auto pru = fr.planes[0];
    auto bywire = u.bywire_map();
    for (int iwire=0; iwire<bywire.size(); ++iwire) {
	cerr << "wire #" << iwire << ":\n";
	auto onewire = bywire[iwire];
	for (int iimp = 0; iimp < onewire.size(); ++iimp) {
	    const int ind = onewire[iimp];
	    double pitch = std::abs(pru.paths[ind].pitchpos);
	    if (iwire < 10) { pitch *= -1.0; }
	    if (iwire == 10 && iimp < onewire.size()/2) { pitch *= -1.0; }
	    if (iimp == 10) { pitch -= 0.001*units::mm; } // roundoff

	    auto wi = u.closest_wire_impact(pitch);
	    //cerr << "\tiimp=" << iimp << " ind=" << ind << " pitch=" << pitch
	    // << " wi.first=" << wi.first << " wi.second=" << wi.second << endl;
	    Assert (iwire == wi.first);
	    Assert (iimp == wi.second);
	}

    }

    for (double pitch=-33*units::mm; pitch <=33*units::mm; pitch += 0.1*units::mm) {
    	auto wi = u.closest_wire_impact(pitch);
    	auto closest = u.closest(pitch);
    	if (closest) {
    	    const Response::Schema::PathResponse& pathr = closest->path_response();
    	    //cerr << "relpitch=" << pitch << " wire=" << wi.first << " imp="
	    // << wi.second << " ->  " << pathr.pitchpos << endl;
	    Assert (std::abs(std::abs(pitch) - std::abs(pathr.pitchpos)) < 0.3*units::mm);
    	}
    }
}

