#include "WireCellUtil/Waveform.h"

#include "WireCellUtil/Units.h"
#include "WireCellUtil/Binning.h"
#include "WireCellUtil/Response.h"

#include "TCanvas.h"
#include "TH1F.h"

#include <algorithm>

using namespace WireCell;

int main()
{
    const double gain_unit = units::mV/units::fC;
    const double gain1 = 7.8*gain_unit;
    const double gain2 = 14.0*gain_unit;
    const double shaping1 = 1.0*units::us;
    const double shaping2 = 2.0*units::us;
    const double tmax = 10.0*units::us;
    const double tick = 0.05*units::us;

    const Binning bins(tmax/tick, 0, tmax);

    Response::ColdElec ce1(gain1, shaping1);
    Response::ColdElec ce2(gain2, shaping2);

    // exercise the generator
    Waveform::realseq_t res1 = ce1.generate(bins);
    Waveform::realseq_t res2 = ce2.generate(bins);

    TH1F resp1("resp1","Cold Electronics Response at 1us shaping",
               bins.nbins(), bins.min()/units::us, bins.max()/units::us);
    TH1F resp2("resp2","Cold Electronics Response at 2us shaping", 
               bins.nbins(), bins.min()/units::us, bins.max()/units::us);
    resp1.SetLineColor(2);
    resp2.SetLineColor(4);
    for (size_t ind=0; ind<res1.size(); ++ind) {
        const double t_us = bins.center(ind) / units::us;
	resp1.Fill(t_us, res1[ind] / gain_unit);
	resp2.Fill(t_us, res2[ind] / gain_unit);
    }

    TCanvas canvas("test_response","Response Functions", 500, 500);
    canvas.SetGridx();
    canvas.SetGridy();
    TH1F* frame = canvas.DrawFrame(0,0,10,15,"Cold Electronics Response Functions (1us,7.8mV/fC and 2us,14.0mV/fC)");
    frame->SetXTitle("Time (microsecond)");
    frame->SetYTitle("Gain (mV/fC)");
    resp1.Draw("hist,same");
    resp2.Draw("hist,same");
    canvas.Print("test_response.pdf");
}
