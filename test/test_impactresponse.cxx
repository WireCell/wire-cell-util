
#include "WireCellUtil/ImpactResponse.h"

#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include "MultiPdf.h"		// local helper shared by a few tests
#include "TH2F.h"
#include "TLine.h"
#include "TStyle.h"

#include <iostream>

using namespace WireCell;
using namespace WireCell::Test;
using namespace std;

void plot_time(MultiPdf& mpdf, PlaneImpactResponse& pir)
{
    auto fr = pir.field_response();
    auto pr = pir.plane_response();
    const int ntsamples = pr.paths[0].current.size();
    const double tstart = fr.tstart*units::us; // fixme, non-standard units
    const double tdelta = fr.period*units::us; // in Response::Schema!
    const double tend = tstart + tdelta * ntsamples;

    const int nwires = pir.nwires();
    const int nimptot = nwires * pir.nimp_per_wire();
    
    const char *uvw = "UVW";
    const int iplane = pr.planeid;

    const double half_pitch = 0.5*pir.pitch_range();
    const double impact_dist = pir.impact();
    
    const double tmin = tend/2.0, tmax=tend;
    const int ntbins = (tmax-tmin)/tdelta;

    const double pmin = -10*units::mm, pmax=10*units::mm;
    const int npbins = (pmax-pmin)/impact_dist;

    gStyle->SetOptStat(0);
    TH2F hist("h",Form("Field Response %c-plane", uvw[iplane]),
	      ntbins, tmin/units::us, tmax/units::us, 
	      npbins, pmin/units::mm, pmax/units::mm);
    hist.SetXTitle("time (us)");
    hist.SetYTitle("pitch (mm)");

    for (double pitch = -half_pitch; pitch <= half_pitch; pitch += impact_dist) {
	auto ir = pir.closest(pitch);
	if (!ir) {
	    std::cerr << "No closest for pitch " << pitch << endl;
	    continue;
	}
	auto wave = ir->waveform();
	pitch += 0.001*impact_dist;
	for (int ind=0; ind<ntsamples; ++ind) {
	    const double time = tstart + (ind+0.001)*tdelta;
	    hist.Fill(time/units::us, pitch/units::mm, wave[ind]);
	}
    }

    hist.Draw("colz");
    mpdf.canvas.SetRightMargin(0.15);

    TLine wline, hline;
    wline.SetLineColorAlpha(1, 0.5);
    wline.SetLineStyle(1);
    hline.SetLineColorAlpha(2, 0.5);
    hline.SetLineStyle(2);
    for (int iwire=0; iwire<nwires/2; ++iwire) {
	double wpitch = iwire * pir.pitch();
	if (wpitch < pmax) { 
	    wline.DrawLine(tmin/units::us, wpitch, tmax/units::us, wpitch);
	    wline.DrawLine(tmin/units::us, -wpitch, tmax/units::us, -wpitch);
	}
	wpitch += 0.5*pir.pitch();
	if (wpitch < pmax) {
	    hline.DrawLine(tmin/units::us, wpitch, tmax/units::us, wpitch);
	    hline.DrawLine(tmin/units::us, -wpitch, tmax/units::us, -wpitch);
	}	    
    }

    mpdf();
}

void test_stuff(Response::Schema::FieldResponse& fr)
{

    PlaneImpactResponse u(fr,0), v(fr,1), w(fr,2);
    
    auto pru = fr.planes[0];
    auto bywire = u.bywire_map();
    for (int iwire=0; iwire<bywire.size(); ++iwire) {
	cerr << "wire #" << iwire << ":\n";
	auto onewire = bywire[iwire];
	for (int iimp = 0; iimp < onewire.size(); ++iimp) {
	    const int ind = onewire[iimp];

	    /// Must doctor pitch since PathResponses are reused by symmetry.
	    double pitch = std::abs(pru.paths[ind].pitchpos);
	    if (iwire < 10) {
		pitch *= -1.0;
	    }
	    if (iwire == 10) {
		if (iimp < onewire.size()/2) {
		    pitch *= -1.0;
		}
	    }
	    if (iimp == 0) {
		pitch += 0.0001*units::mm; // sukoshi choto
	    }
	    if (iimp == onewire.size()-1) {
		pitch -= 0.0001*units::mm; // sukoshi choto
	    }

	    cerr << "\tiimp=" << iimp << ", ind=" << ind << ", pitch=" << pitch << endl;
	    auto wi = u.closest_wire_impact(pitch);
	    cerr << "\tiimp=" << iimp << " ind=" << ind << " pitch=" << pitch
		 << " closest wire=" << wi.first << " closest impact=" << wi.second << endl;
	    Assert (iwire == wi.first);
	    Assert (iimp == wi.second);
	}

    }

    for (double pitch=-33*units::mm; pitch <=33*units::mm; pitch += 0.1*units::mm) {
    	auto wi = u.closest_wire_impact(pitch);
    	auto closest = u.closest(pitch);
    	if (closest) {
    	    const Response::Schema::PathResponse& pathr = closest->path_response();
    	    cerr << "relpitch=" << pitch << " wire=" << wi.first << " imp="
		 << wi.second << " ->  " << pathr.pitchpos << endl;
	    Assert (std::abs(std::abs(pitch) - std::abs(pathr.pitchpos)) < 0.3*units::mm);
    	}
	else {
	    std::cerr << "No closest for pitch " << pitch << std::endl;
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

    // 1D garfield wires are all parallel
    const double angle = 60*units::degree;
    Vector upitch(0, -sin(angle),  cos(angle));
    Vector uwire (0,  cos(angle),  sin(angle));
    Vector vpitch(0,  cos(angle),  sin(angle));
    Vector vwire (0, -sin(angle),  cos(angle));
    Response::Schema::lie(fr.planes[0], upitch, uwire);
    Response::Schema::lie(fr.planes[1], vpitch, vwire);


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
    mpdf.close();
    return 0;

}


