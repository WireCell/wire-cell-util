#include "WireCellUtil/Waveform.h"

#include "WireCellUtil/Response.h"

#include "TCanvas.h"
#include "TH1F.h"

#include <iostream>
#include <algorithm>

using namespace std;
using namespace WireCell;

void draw_time_freq(TCanvas& canvas,
		    Waveform::realseq_t& res, 
		    const std::string& title, 
		    const Waveform::Domain& domain = Waveform::Domain(0.0,10.0));
void draw_time_freq(TCanvas& canvas,
		    Waveform::realseq_t& res, 
		    const std::string& title, 
		    const Waveform::Domain& domain)
{
    Waveform::compseq_t spec = Waveform::dft(res);
    Waveform::realseq_t res2 = Waveform::idft(spec);

    int nticks = res.size();
    TH1F h_wave("response",title.c_str(), nticks, domain.first, domain.second);
    TH1F h_wave2("response2",title.c_str(), nticks, domain.first, domain.second);
    h_wave2.SetLineColor(2);

    h_wave.SetXTitle("Time (microsecond)");
    h_wave.SetYTitle("Gain (mV/fC)");

    for (int ind=0; ind<nticks; ++ind) {
	h_wave.SetBinContent(ind+1, res[ind]);
	h_wave2.SetBinContent(ind+1, res2[ind]);
    }

    cerr << nticks << " " << spec.size() << endl;

    const double tick = (domain.second-domain.first)/nticks;

    TH1F h_mag("mag","Magnitude of Fourier transform of response", nticks, 0, 1/tick);
    h_mag.SetYTitle("Power");
    h_mag.SetXTitle("MHz");

    TH1F h_phi("phi","Phase of Fourier transform of response", nticks, 0, 1/tick);
    h_phi.SetYTitle("Power");
    h_phi.SetXTitle("MHz");

    for (int ind=0; ind<nticks; ++ind) {
	auto c = spec[ind];
	h_mag.SetBinContent(ind+1, std::abs(c));
	h_phi.SetBinContent(ind+1, std::arg(c));
    }

    canvas.Clear();
    canvas.Divide(2,1);

    auto pad = canvas.cd(1);
    pad->SetGridx();
    pad->SetGridy();
    h_wave.Draw();
    h_wave2.Draw("same");

    auto spad = canvas.cd(2);
    spad->Divide(1,2);
    pad = spad->cd(1);
    pad->SetGridx();
    pad->SetGridy();
    h_mag.Draw();

    pad = spad->cd(2);
    pad->SetGridx();
    pad->SetGridy();
    h_phi.Draw();
    canvas.Print("test_fft.pdf",".pdf");

}

int main()
{
    const std::vector<double> gains = {7.8, 14.0}; // mV/fC
    const std::vector<double> shapings = {1.0, 2.0}; // microsecond

    const Waveform::Domain domain(0.0, 10.0);
    const double tick = 0.5;
    const int nticks = Waveform::sample_count(domain, tick);
    

    const double tconst = 1000.0; // 1ms

    TCanvas canvas("test_fft", "Response Functions", 500, 500);
    canvas.Print("test_fft.pdf[",".pdf");

    for (int ind=0; ind<gains.size(); ++ind) {
	Response::ColdElec ce(gains[ind], shapings[ind]);
	Waveform::realseq_t res = ce.generate(domain, nticks);

	draw_time_freq(canvas, res,
		       Form("Cold Electronics Response at %.0fus shaping", shapings[ind]), domain);
    }


    {
	Response::SimpleRC rc(tconst);
	Waveform::realseq_t res = rc.generate(domain, nticks);
	
	draw_time_freq(canvas, res,
		       "RC Response at 1ms time constant", domain);
    }
    {
	Waveform::Domain shifted = domain;
	shifted.first += tick;	// intentionally miss delta function
	shifted.second += tick;

	Response::SimpleRC rc(tconst); 
	Waveform::realseq_t res = rc.generate(shifted, nticks);
	
	draw_time_freq(canvas, res,
		       "RC Response at 1ms time constant (suppress delta)", domain);
    }

    canvas.Print("test_fft.pdf]",".pdf");
}
