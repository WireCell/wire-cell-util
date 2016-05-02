#include "WireCellUtil/Waveform.h"

using namespace WireCell;

std::pair<double,double> WireCell::Waveform::mean_rms(timeseq_t& wf)
{
    int n = wf.size();
    if (n==0) {
	return std::make_pair<double,double>(0,0);
    }
    if (n==1) {
	return std::make_pair<double,double>(wf(0),0);
    }
    double mean = wf.mean();
    double rms = sqrt(wf.matrix().squaredNorm()/n - mean*mean);
    return std::make_pair(mean,rms);
}


Waveform::freqseq_t WireCell::Waveform::fft(timeseq_t& wave)
{
    Eigen::FFT<float> trans;
    Eigen::VectorXcf ret = trans.fwd(wave.matrix());
    return ret.array();
}

Waveform::timeseq_t WireCell::Waveform::ifft(freqseq_t& spec)
{
    // fixme: how about you write something here?
}
