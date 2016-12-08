
#include "WireCellUtil/ImpactResponse.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include "MultiPdf.h"		// local helper shared by a few tests
#include "TH2F.h"

#include <iostream>

using namespace WireCell;
using namespace WireCell::Test;
using namespace std;

void plot_time(MultiPdf& mpdf, PlaneImpactResponse& pir)
{
    auto fr = pir.field_response();
    auto pr = pir.plane_response();
    const int nsamples = pr.paths[0].current.size();
    const double tstart = fr.tstart;
    const double tend = tstart + fr.period * nsamples;

    const int nwires = pir.nwires();
    const int nimptot = nwires * pir.nimp_per_wire();
    
    const char *uvw = "UVW";
    const int iplane = pr.planeid;

    const double half_pitch = pir.half_pitch();
    const double impact_dist = pir.impact();

    TH2F hist("h",Form("Pitch vs Time %c-plane", uvw[iplane]),
	      nsamples, tstart, tend,
	      nimptot, -half_pitch, half_pitch);

    for (double pitch = -half_pitch; pitch <= half_pitch; pitch += impact_dist) {
	auto ir = pir.closest(pitch);
	auto wave = ir->waveform();
	for (int ind=0; ind<nsamples; ++ind) {
	    const double time = tstart + ind*fr.period;
	    hist.Fill(time, pitch, wave[ind]);
	}
    }

    hist.Draw("colz");
    mpdf();
}

void test_stuff(Response::Schema::FieldResponse& fr)
{

    PlaneImpactResponse u(fr,0), v(fr,1), w(fr,2);
    
    auto pru = fr.planes[0];
    auto bywire = u.bywire_map();
    for (int iwire=0; iwire<bywire.size(); ++iwire) {
	//cerr << "wire #" << iwire << ":\n";
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


int main(int argc, const char* argv[])
{

    if (argc < 2) {
	cerr << "This test requires an Wire Cell Field Response input file." << endl;
	return 0;
    }

    WireCell::ExecMon em(argv[0]);
    auto fr = Response::Schema::load(argv[1]);
    em("loaded");

    MultiPdf mpdf(argv[0]);

    test_stuff(fr);
    for (int iplane=0; iplane<3; ++iplane) {
	PlaneImpactResponse pir(fr,iplane);
	Assert(iplane == pir.plane_response().planeid);

	auto pr = fr.planes[iplane];
	Assert(iplane == pr.planeid);

	cerr << "plane: " << iplane << " #paths:" << pr.paths.size() << " pitchv=" << pr.pitchdir << endl;

	plot_time(mpdf, pir);
    }

    cerr << em.summary() << endl;
    return 0;

}


