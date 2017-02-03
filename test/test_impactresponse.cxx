#include "WireCellUtil/PlaneImpactResponse.h"

#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include "MultiPdf.h"		// local helper shared by a few tests
#include "TH2F.h"
#include "TLine.h"
#include "TStyle.h"
#include "TFile.h"

#include <iostream>

using namespace WireCell;
using namespace WireCell::Test;
using namespace std;

void plot_time(MultiPdf& mpdf, PlaneImpactResponse& pir,
               const std::string& name, const std::string& title)
{
    auto fr = pir.field_response();
    auto pr = pir.plane_response();
    Binning tbins = pir.tbins();

    // only show bins where we think the response is
    const double tmin = tbins.min();
    const double tspan = 100*units::us;
    const int ntbins = tbins.bin(tmin+tspan);
    const double tmax = tbins.edge(ntbins);

    const int nwires = pir.nwires();
    const int nimptot = nwires * pir.nimp_per_wire();
    
    const char *uvw = "UVW";
    const int iplane = pr.planeid;

    const double half_pitch = 0.5*pir.pitch_range();
    const double impact_dist = pir.impact();

    const double pmin = -36*units::mm, pmax=36*units::mm;
    const int npbins = (pmax-pmin)/impact_dist;

    gStyle->SetOptStat(0);
    TH2F* hist = new TH2F(Form("h%s_%c", name.c_str(), uvw[iplane]),
                          Form("%s %c-plane", title.c_str(), uvw[iplane]),
                          ntbins, tmin/units::us, tmax/units::us, 
                          npbins, pmin/units::mm, pmax/units::mm);
    hist->SetXTitle("time (us)");
    hist->SetYTitle("pitch (mm)");

    for (double pitch = -half_pitch; pitch <= half_pitch; pitch += impact_dist) {
	auto ir = pir.closest(pitch);
	if (!ir) {
	    std::cerr << "No closest for pitch " << pitch << endl;
	    continue;
	}
        auto spec = ir->spectrum();
	auto wave = Waveform::idft(spec);
	pitch += 0.001*impact_dist;
	for (int ind=0; ind < ntbins; ++ind) {
	    const double time = tbins.center(ind);
	    hist->Fill(time/units::us, pitch/units::mm, wave[ind]);
	}
    }
    hist->Write();
    hist->Draw("colz");
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

void test_stuff(Response::Schema::FieldResponse& fr, int iplane)
{
    const int ntbins = 10000;
    const double tmin = 0.0;
    const double tmax = 5*units::ms;
    Binning tbins(ntbins, tmin, tmax);
    const double gain = 14.7;
    const double shaping = 2*units::us;

    PlaneImpactResponse pir(fr, iplane, tbins, gain, shaping);
    auto& pr = fr.planes[iplane];
    auto bywire = pir.bywire_map();

    for (int iwire=0; iwire<bywire.size(); ++iwire) {

        auto onewire = bywire[iwire];
        for (int iimp = 0; iimp < onewire.size(); ++iimp) {
            const int ind = onewire[iimp];

            /// Must doctor pitch since PathResponses are reused by symmetry.
            double pitch = std::abs(pr.paths[ind].pitchpos);
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

            auto wi = pir.closest_wire_impact(pitch);
            Assert (iwire == wi.first);
            Assert (iimp == wi.second);
        }
            
    } // loop over wires

    for (double pitch=-33*units::mm; pitch <=33*units::mm; pitch += 0.1*units::mm) {
        auto wi = pir.closest_wire_impact(pitch);
        auto closest = pir.closest(pitch);
        if (closest) {
            int impact = closest->impact();
            const Response::Schema::PathResponse& pathr = pr.paths[impact];
            Assert (std::abs(std::abs(pitch) - std::abs(pathr.pitchpos)) < 0.3*units::mm);
        }
        else {
            std::cerr << "No closest for pitch " << pitch << std::endl;
        }
    } // pitches get stitches

}


int main(int argc, const char* argv[])
{

    if (argc < 2) {
	cerr << "This test requires an Wire Cell Field Response input file." << endl;
	return 0;
    }
    string out_basename = argv[0];
    if (argc > 2) {
        out_basename = argv[2];
    }
    TFile* rootfile = TFile::Open(Form("%s.root", out_basename.c_str()), "recreate");

    WireCell::ExecMon em(argv[0]);
    auto fr = Response::Schema::load(argv[1]);
    cerr << em("loaded") << endl;

    // 1D garfield wires are all parallel
    const double angle = 60*units::degree;
    Vector upitch(0, -sin(angle),  cos(angle));
    Vector uwire (0,  cos(angle),  sin(angle));
    Vector vpitch(0,  cos(angle),  sin(angle));
    Vector vwire (0, -sin(angle),  cos(angle));
    Response::Schema::lie(fr.planes[0], upitch, uwire);
    Response::Schema::lie(fr.planes[1], vpitch, vwire);


    for (int iplane=0; iplane<3; ++iplane) {
//        test_stuff(fr, iplane);
    }

    const int ntbins = 10000;
    const double tmin = 0.0;
    const double tmax = 5*units::ms;
    Binning tbins(ntbins, tmin, tmax);
    const double gain = 14.7;
    const double shaping = 2*units::us;

    MultiPdf mpdf(argv[0]);
    for (int iplane=0; iplane<3; ++iplane) {
	PlaneImpactResponse pir_fonly(fr, iplane, tbins);
	plot_time(mpdf, pir_fonly, "fr", "Field Response");

	PlaneImpactResponse pir_elec(fr, iplane, tbins, gain, shaping);
	plot_time(mpdf, pir_elec, "dr", "Detector Response");
    }

    cerr << em.summary() << endl;
    mpdf.close();

    cerr << "Closing ROOT file: " << rootfile->GetName() << endl;
    rootfile->Close();
    
    return 0;

}


