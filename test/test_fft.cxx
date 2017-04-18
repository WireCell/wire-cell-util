// fixme: this test utterly fails to properly use the system of units!

#include "WireCellUtil/Waveform.h"

#include "WireCellUtil/Response.h"

#include "TCanvas.h"
#include "TH1F.h"
#include "TLine.h"
#include "TText.h"

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

    const int nticks = res.size();
    cerr << "Drawing nticks=" << nticks << endl;

    TH1F h_wave("response",title.c_str(), nticks, domain.first, domain.second);
    TH1F h_wave2("response2",title.c_str(), nticks, domain.first, domain.second);
    h_wave2.SetLineColor(2);

    h_wave.SetXTitle("Time (microsecond)");
    h_wave.SetYTitle("Gain (mV/fC)");
    h_wave.GetXaxis()->SetRangeUser(0,10.0);

    for (int ind=0; ind<nticks; ++ind) {
	h_wave.SetBinContent(ind+1, res[ind]);
	h_wave2.SetBinContent(ind+1, res2[ind]);
    }

    cerr << nticks << " " << spec.size() << endl;

    const double tick = (domain.second-domain.first)/nticks;

    const double max_freq = 1/tick;
    TH1F h_mag("mag","Magnitude of Fourier transform of response", nticks, 0, max_freq);
    h_mag.SetYTitle("Amplitude");
    h_mag.SetXTitle("MHz");

    TH1F h_phi("phi","Phase of Fourier transform of response", nticks, 0, max_freq);
    h_phi.SetYTitle("Amplitude");
    h_phi.SetXTitle("MHz");

    for (int ind=0; ind<nticks; ++ind) {
	auto c = spec[ind];
        const double freq = tick*ind;
	h_mag.SetBinContent(ind+1, std::abs(c));
	h_phi.SetBinContent(ind+1, std::arg(c));
    }
    h_mag.GetXaxis()->SetRangeUser(0,2.0);
    h_phi.GetXaxis()->SetRangeUser(0,2.0);

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

    TLine ndb;
    TText ndbtxt;
    double maxmag = h_mag.GetMaximum();
    double db3 = maxmag / 1.414;
    double db6 = maxmag / 1.995;
    double db10 = maxmag / 3.162;
    double db20 = maxmag / 10;
    ndb.DrawLine(0,db3, 1.0, db3);
    ndb.DrawLine(0,db6,   1.0, db6);
    ndb.DrawLine(0,db10,   1.0, db10);
    ndb.DrawLine(0,db20,   1.0, db20);
    ndbtxt.DrawText(0.5, db3, "-3dB");
    ndbtxt.DrawText(0.5, db6, "-6dB");    
    ndbtxt.DrawText(0.5, db10, "-10dB");    
    ndbtxt.DrawText(0.5, db20, "-20dB");    

    pad = spad->cd(2);
    pad->SetGridx();
    pad->SetGridy();
    h_phi.Draw();
    canvas.Print("test_fft.pdf","pdf");

}

int main()
{
    const std::vector<double> gains = {7.8, 14.0}; // mV/fC
    const std::vector<double> shapings = {1.0, 2.0}; // microsecond

    const Waveform::Domain domain(0.0, 100.0); // us
    const double tick = 0.1;    // us
    const int nticks = Waveform::sample_count(domain, tick);
    cerr << "Using nticks=" << nticks << endl;

    const double tconst = 1000.0; // ms

    TCanvas canvas("test_fft", "Response Functions", 500, 500);
    canvas.Print("test_fft.pdf[","pdf");

    for (int ind=0; ind<gains.size(); ++ind) {
	Response::ColdElec ce(gains[ind], shapings[ind]);
	Waveform::realseq_t res = ce.generate(domain, nticks);

	draw_time_freq(canvas, res,
		       Form("Cold Electronics Response at %.0fus peaking", shapings[ind]), domain);
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

    canvas.Print("test_fft.pdf]","pdf");
}
