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

// The preferred display units for gain.
const double GUnit = units::mV/units::fC;


void draw_time_freq(TCanvas& canvas,
		    Waveform::realseq_t& res, 
		    const std::string& title, 
                    const Binning& tbins)
{
    Waveform::compseq_t spec = Waveform::dft(res);
    Waveform::realseq_t res2 = Waveform::idft(spec);

    const int nticks = res.size();
    cerr << "Drawing nticks=" << nticks << endl;

    TH1F h_wave("response",title.c_str(),
                tbins.nbins(), tbins.min()/units::us, tbins.max()/units::us);
    TH1F h_wave2("response2",title.c_str(),
                 tbins.nbins(), tbins.min()/units::us, tbins.max()/units::us);
    h_wave2.SetLineColor(2);

    h_wave.SetXTitle("Time (microsecond)");
    h_wave.SetYTitle("Gain (mV/fC)");
    h_wave.GetXaxis()->SetRangeUser(0,10.0);

    for (int ind=0; ind<nticks; ++ind) {
        h_wave.Fill(tbins.center(ind)/units::us, res[ind]/GUnit);
        h_wave2.Fill(tbins.center(ind)/units::us, res2[ind]/GUnit);
    }

    cerr << nticks << " " << spec.size() << endl;

    const double tick = tbins.binsize();
    const Binning fbins(nticks, 0, 1/tick);

    TH1F h_mag("mag","Magnitude of Fourier transform of response",
               fbins.nbins(), fbins.min()/units::megahertz, fbins.max()/units::megahertz);
    h_mag.SetYTitle("Amplitude [mV/fC]");
    h_mag.SetXTitle("MHz");

    TH1F h_phi("phi","Phase of Fourier transform of response",
               fbins.nbins(), fbins.min()/units::megahertz, fbins.max()/units::megahertz);
    h_phi.SetYTitle("Phase [radian]");
    h_phi.SetXTitle("MHz");

    for (int ind=0; ind<nticks; ++ind) {
	auto c = spec[ind];
        const double freq = fbins.center(ind);
        h_mag.Fill(freq/units::megahertz, std::abs(c)/GUnit);
        h_phi.Fill(freq/units::megahertz, std::arg(c));
    }
    //h_mag.GetXaxis()->SetRangeUser(0,2.0);
    //h_phi.GetXaxis()->SetRangeUser(0,2.0);

    canvas.Clear();
    canvas.Divide(2,1);

    auto pad = canvas.cd(1);
    pad->SetGridx();
    pad->SetGridy();
    h_wave.Draw("hist");
    h_wave2.Draw("hist,same");

    auto spad = canvas.cd(2);
    spad->Divide(1,2);
    pad = spad->cd(1);
    pad->SetGridx();
    pad->SetGridy();
    h_mag.Draw("hist");

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
    h_phi.Draw("hist");
    canvas.Print("test_fft.pdf","pdf");

}

int main()
{
    const std::vector<double> gains = {7.8*GUnit, 14.0*GUnit};
    const std::vector<double> shapings = {1.0*units::us, 2.0*units::us};

    const double maxtime = 100.0*units::us;
    const double tick = 0.5*units::us;
    const Binning tbins(maxtime/tick, 0, maxtime);
    const int nticks = tbins.nbins();
    cerr << "Using nticks=" << nticks << endl;

    TCanvas canvas("test_fft", "Response Functions", 500, 500);
    canvas.Print("test_fft.pdf[","pdf");

    for (int ind=0; ind<gains.size(); ++ind) {
	Response::ColdElec ce(gains[ind], shapings[ind]);
	Waveform::realseq_t res = ce.generate(tbins);

        const double tshape_us = shapings[ind]/units::us;
	draw_time_freq(canvas, res,
		       Form("Cold Electronics Response at %.0fus peaking", tshape_us), tbins);
    }


    // Look at RC filter

    const double tconst = 1000.0*units::ms;

    {
	Response::SimpleRC rc(tconst);
	Waveform::realseq_t res = rc.generate(tbins);
	
	draw_time_freq(canvas, res,
		       "RC Response at 1ms time constant", tbins);
    }
    {
        Binning shifted(tbins.nbins(), tbins.min()+tick, tbins.max()+tick);

	Response::SimpleRC rc(tconst); 
	Waveform::realseq_t res = rc.generate(shifted);
	
	draw_time_freq(canvas, res,
		       "RC Response at 1ms time constant (suppress delta)", tbins);
    }

    canvas.Print("test_fft.pdf]","pdf");
}
