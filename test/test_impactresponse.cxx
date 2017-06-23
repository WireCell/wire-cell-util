
#include "WireCellUtil/PlaneImpactResponse.h"

#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include "MultiPdf.h"		// local helper shared by a few tests
#include "TH2F.h"
#include "TLine.h"
#include "TStyle.h"
#include "TFile.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include "Utils.h"

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
    //const int nimptot = nwires * pir.nimp_per_wire();
    
    const char *uvw = "UVW";
    const int iplane = pr.planeid;

    const double half_pitch = 0.5*pir.pitch_range();
    const double impact_dist = pir.impact();

    const double pmin = -36*units::mm, pmax=36*units::mm;
    const int npbins = (pmax-pmin)/impact_dist;

    // dr:
    std::string zunit = "negative microvolt";
    double zunitval = -units::microvolt;
    vector<double> zextent{1.0, 1.0, 2.0};
    if (name=="fr") {
        zunit = "induced electrons";
        zunitval = -units::eplus;
        zextent = vector<double>{0.3, 0.15, 0.6};
    }
    std::cerr <<"zunits: " << zunit 
              << " tbinsize: " << tbins.binsize()/units::us
              << " us\n";


    // they all suck.  black body sucks the least.
    set_palette(kBlackBody);
    //set_palette(kLightTemperature);
    //set_palette(kRedBlue);
    //set_palette(kTemperatureMap);
    //set_palette(kThermometer);
    // set_palette(kVisibleSpectrum);
    //set_palette();
    gStyle->SetOptStat(0);
    TH2F* hist = new TH2F(Form("h%s_%c", name.c_str(), uvw[iplane]),
                          Form("%s, 1e-/impact %c-plane", title.c_str(), uvw[iplane]),
                          ntbins, tmin/units::us, tmax/units::us, 
                          npbins, pmin/units::mm, pmax/units::mm);
    hist->SetXTitle("time (us)");
    hist->SetYTitle("pitch (mm)");
    hist->SetZTitle(zunit.c_str());

    hist->GetZaxis()->SetRangeUser(-zextent[iplane], +zextent[iplane]);

    TH1F* htot = new TH1F(Form("htot%s_%c", name.c_str(), uvw[iplane]),
                          Form("%s total, 1e-/impact %c-plane", title.c_str(), uvw[iplane]),
                          npbins, pmin/units::mm, pmax/units::mm);
    htot->SetXTitle("pitch (mm)");
    htot->SetYTitle(Form("impact total [%s]", zunit.c_str()));

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
	    hist->Fill(time/units::us, pitch/units::mm, wave[ind]/zunitval);
            htot->Fill(pitch/units::mm, wave[ind]/zunitval);
	}
    }
    hist->Write();
    hist->Draw("colz");

    mpdf.canvas.SetRightMargin(0.15);
    mpdf.canvas.SetLeftMargin(0.15);

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
    htot->Draw("hist");
    mpdf();

}

void test_stuff(Response::Schema::FieldResponse& fr, int iplane)
{
    const int ntbins = 10000;
    const double tmin = 0.0;
    const double tmax = 5*units::ms;
    Binning tbins(ntbins, tmin, tmax);
    const double gain = 14.0*units::mV/units::fC;
    const double shaping = 2*units::us;

    PlaneImpactResponse pir(fr, iplane, tbins, gain, shaping);
    auto& pr = fr.planes[iplane];
    auto bywire = pir.bywire_map();

    for (size_t iwire=0; iwire<bywire.size(); ++iwire) {

        auto onewire = bywire[iwire];
        for (size_t iimp = 0; iimp < onewire.size(); ++iimp) {
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
            Assert ((int)iwire == wi.first);
            Assert ((int)iimp == wi.second);
        }
            
    } // loop over wires

    for (double pitch=-33*units::mm; pitch <=33*units::mm; pitch += 0.1*units::mm) {
        //auto wi = pir.closest_wire_impact(pitch);
        auto closest = pir.closest(pitch);
        if (closest) {
            int impact = closest->impact();
            const Response::Schema::PathResponse& pathr = pr.paths[impact];
            Assert (std::abs(std::abs(pitch) - std::abs(pathr.pitchpos)) < 0.3*units::mm);
            if (pitch >= -1.5*units::mm and pitch <= 1.5*units::mm) {
                auto& wave = pathr.current;
                const double itot = Waveform::sum(wave);
                const double qtot = itot * fr.period;
                auto mm = std::minmax_element(wave.begin(), wave.end());
                
                std::cerr << "plane: " << iplane << " pitch:" << pitch
                          << " itot=" << itot << " "
                          << " Imm=["
                          << (*mm.first)/units::nanoampere << ","
                          << (*mm.second)/units::nanoampere <<"]nAmp "
                          << " qtot=" << qtot/units::eplus << "eles\n";
            }
        }
        else {
            std::cerr << "No closest for pitch " << pitch << std::endl;
        }
    } // pitches get stitches

}


int main(int argc, const char* argv[])
{
    string response_file = "garfield-1d-3planes-21wires-6impacts-v6.json.bz2";
    if (argc < 2) {
	cerr << "No Wire Cell field response input file given, will try to use:\n"
             << response_file << endl;
    }
    else {
        response_file = argv[1];
    }
    string out_basename = argv[0];
    if (argc > 2) {
        out_basename = argv[2];
    }
    TFile* rootfile = TFile::Open(Form("%s.root", out_basename.c_str()), "recreate");

    WireCell::ExecMon em(argv[0]);
    auto fr = Response::Schema::load(response_file.c_str());
    em("loaded");

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
    const double gain = 14.0*units::mV/units::fC;
    const double shaping = 2*units::us;

    MultiPdf mpdf(out_basename.c_str());
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


