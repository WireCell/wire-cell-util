#include "WireCellUtil/Waveform.h"

#include "WireCellUtil/Response.h"

#include "TCanvas.h"
#include "TH1F.h"

#include <algorithm>

using namespace WireCell;

int main()
{
    const double gain_par1 = 7.8; // mV/fC
    const double gain_par2 = 14.0;// mV/fC
    const double shaping1 = 1.0; // microsecond
    const double shaping2 = 2.0; // microsecond
    const Waveform::Domain domain(0.0,10.0); // microsecond
    const double tick=0.05; // microsecond
    const int nticks = (domain.second-domain.first)/tick;

    Response::ColdElec ce1(gain_par1, shaping1);
    Response::ColdElec ce2(gain_par2, shaping2);

    // exercise the generator
    Waveform::realseq_t res1 = ce1.generate(domain, nticks);
    Waveform::realseq_t res2 = ce2.generate(domain, nticks);

    TH1F resp1("resp1","Cold Electronics Response at 1us shaping", nticks, domain.first, domain.second);
    TH1F resp2("resp2","Cold Electronics Response at 2us shaping", nticks, domain.first, domain.second);
    resp1.SetLineColor(2);
    resp2.SetLineColor(4);
    for (int ind=0; ind<res1.size(); ++ind) {
	resp1.SetBinContent(ind+1, res1[ind]);
	resp2.SetBinContent(ind+1, res2[ind]);
    }

    TCanvas canvas("test_response","Response Functions", 500, 500);
    canvas.SetGridx();
    canvas.SetGridy();
    TH1F* frame = canvas.DrawFrame(0,0,10,15,"Cold Electronics Response Functions (1us,7.8mV/fC and 2us,14.0mV/fC)");
    frame->SetXTitle("Time (microsecond)");
    frame->SetYTitle("Gain (mV/fC)");
    resp1.Draw("same");
    resp2.Draw("same");
    canvas.Print("test_response.pdf");
}
