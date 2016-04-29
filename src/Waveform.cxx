#include "WireCellUtil/Waveform.h"

using namespace WireCell;

std::pair<double,double> WireCell::Waveform::mean_rms(signal_t& wf)
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


Waveform::fourier_t WireCell::Waveform::fft(signal_t& wave)
{
    Eigen::FFT<float> trans;
    Eigen::VectorXcf ret = trans.fwd(wave.matrix());
    return ret.array();
}

Waveform::signal_t WireCell::Waveform::ifft(fourier_t& spec)
{
}
