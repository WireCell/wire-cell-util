#include "WireCellUtil/Waveform.h"

// for FFT
#include <Eigen/Core>
#include <unsupported/Eigen/FFT>


using namespace WireCell;


std::pair<int,int>
WireCell::Waveform::sub_sample(const Waveform::Domain& domain, int nsamples, const Waveform::Domain& subdomain)
{
    const double bin = sample_width(domain, nsamples);
    int beg = ceil((subdomain.first - domain.first) / bin);
    int end = nsamples- ceil((domain.second - subdomain.second) / bin);
    return std::make_pair(std::max(0,beg), std::min(nsamples, end));
}

std::pair<double,double>
WireCell::Waveform::mean_rms(const realseq_t& wf)
{
    int n = wf.size();
    if (n==0) {
	return std::make_pair<double,double>(0,0);
    }
    if (n==1) {
	return std::make_pair<double,double>(wf[0],0);
    }

    double mean = Waveform::sum(wf)/n;
    double rms = sqrt(Waveform::sum2(wf)/n - mean*mean);
    return std::make_pair(mean,rms);
}


template<class Func>
Waveform::realseq_t c2r(const Waveform::compseq_t& seq, Func func)
{
    Waveform::realseq_t ret(seq.size());
    std::transform(seq.begin(), seq.end(), ret.begin(), func);
    return ret;
}

Waveform::realseq_t WireCell::Waveform::real(const Waveform::compseq_t& seq)
{
    return c2r(seq, [](Waveform::complex_t c) { return std::real(c); });
}

Waveform::realseq_t WireCell::Waveform::imag(const Waveform::compseq_t& seq)
{
    return c2r(seq, [](Waveform::complex_t c) { return std::imag(c); });
}

Waveform::realseq_t WireCell::Waveform::magnitude(const Waveform::compseq_t& seq)
{
    return c2r(seq, [](Waveform::complex_t c) { return std::abs(c); });
}

Waveform::realseq_t WireCell::Waveform::phase(const Waveform::compseq_t& seq)
{
    return c2r(seq, [](Waveform::complex_t c) { return std::arg(c); });
}


Waveform::real_t WireCell::Waveform::median(Waveform::realseq_t wave)
{
    std::sort(wave.begin(), wave.end());
    return wave[wave.size()/2];
}

Waveform::compseq_t WireCell::Waveform::dft(realseq_t wave)
{
    auto v = Eigen::Map<Eigen::VectorXf>(wave.data(), wave.size());
    Eigen::FFT<float> trans;
    Eigen::VectorXcf ret = trans.fwd(v);
    return compseq_t(ret.data(), ret.data()+ret.size());
}

Waveform::realseq_t WireCell::Waveform::idft(compseq_t spec)
{
    Eigen::FFT<float> trans;
    auto v = Eigen::Map<Eigen::VectorXcf>(spec.data(), spec.size());
    Eigen::VectorXf ret;
    trans.inv(ret, v);
    return realseq_t(ret.data(), ret.data()+ret.size());
}

