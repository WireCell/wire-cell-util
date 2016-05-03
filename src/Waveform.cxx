#include "WireCellUtil/Waveform.h"

using namespace WireCell;

std::pair<double,double> WireCell::Waveform::mean_rms(realseq_t& wf)
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


Waveform::compseq_t WireCell::Waveform::dft(realseq_t& wave)
{
    Eigen::FFT<float> trans;
    Eigen::VectorXcf ret = trans.fwd(wave.matrix());
    return ret.array();
}

Waveform::realseq_t WireCell::Waveform::idft(compseq_t& spec)
{
    Eigen::FFT<float> trans;
    Eigen::VectorXf ret;
    trans.inv(ret, spec.matrix());
    return ret.array();
}


Waveform::realseq_t WireCell::Waveform::real(const Waveform::compseq_t& seq)
{
    return seq.real();
}

Waveform::realseq_t WireCell::Waveform::imag(const Waveform::compseq_t& seq)
{
    return seq.imag();
}

Waveform::realseq_t WireCell::Waveform::magnitude(const Waveform::compseq_t& seq)
{
    return seq.abs();
}


static  Waveform::real_t myarg(const Waveform::complex_t& x)
{
    return std::arg(x);
}
Waveform::realseq_t WireCell::Waveform::phase(const Waveform::compseq_t& seq)
{
    // Apparently arg() isn't implemented. 

    return seq.unaryExpr(ptr_fun(myarg));
}

Waveform::real_t WireCell::Waveform::median(Waveform::realseq_t& wave)
{
    std::vector<float> v = Waveform::eig2std(wave);
    std::sort(v.begin(), v.end());
    return v[v.size()/2];
}


