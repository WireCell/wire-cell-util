#include "WireCellUtil/Pimpos.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

using namespace std;
using namespace WireCell;

int main()
{
    const double pitch_dist = 3*units::mm;
    const int nimperw = 10;
    const double impdist = pitch_dist/nimperw;
    const double gap = 3*units::mm;
    const double extent = 5*units::m;
    const Point origin(0.0, 0.0, -1.0*extent);
    const Vector wire_dir(0, 1, 0);
    const Vector pitch_dir(0, 0, 1);
    const Vector pitch_vec(0, 0, pitch_dist);
    const Point zero(0,0,0);
    const Point onetwothree(1.0*units::cm, 2.0*units::cm, 3.0*units::cm);

    Pimpos pimpos(origin, wire_dir, pitch_vec, nimperw);

    for (int ind=0; ind<3; ++ind) {
	cerr << ind << ": " << pimpos.axis(ind) << endl;
    }

    {
	auto val = pimpos.relative(zero);
	cerr << "relative: " << val << endl;
	Assert(val == -1.0*origin);
    }

    {
	auto val = pimpos.distance(zero, 0);
	cerr << "distance 0: " << val << endl;
	Assert (val == 0.0);
    }

    {
	auto val = pimpos.distance(zero, 2);
	cerr << "distance 2: " << val << endl;
	Assert (val == extent);
    }

    {
	auto v = pimpos.transform(onetwothree);
	cerr << "v123: " << v << endl;
    }

    {
	const int nwire = 4;
	const int relimp = 3;
	const int wantimp = nwire*nimperw + relimp;
	Assert(pimpos.absolute_impact(nwire,relimp) == wantimp);

	auto wi = pimpos.wire_impact(wantimp);
	Assert(wi.first == nwire && wi.second == relimp);

	const double wantpit = wantimp * impdist;
	const double givepit = wantpit + 0.1*impdist;

	auto gotimp = pimpos.impact(givepit);
	cerr << "gave: " << givepit/impdist << " got:" << gotimp << " want: " << wantimp << endl;
	Assert (gotimp == wantimp);
	auto gotpit = pimpos.pitch(gotimp);
	Assert (gotpit == wantpit);

	
    }

    {
	int iwire = 100;
	for (int iimp = -3*nimperw; iimp <3*nimperw; ++iimp) {
	    //auto ii = pimpos.reflect(iwire, iimp);
	    auto absimp = pimpos.absolute_impact(iwire, iimp);
	    auto ii = pimpos.wire_impact(absimp);
	    cerr << absimp << ": " << iwire << ":" << iimp << " --> " << ii.first << ":" << ii.second << endl;
	}
    }



    return 0;

}
