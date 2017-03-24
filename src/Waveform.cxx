#include "WireCellUtil/Waveform.h"

#include <algorithm>

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
    //return percentile(wave,0.5);
}

#include <iostream>
Waveform::real_t WireCell::Waveform::median_binned(Waveform::realseq_t& wave)
{
    return percentile_binned(wave,0.5);
    // const auto mm = std::minmax_element(wave.begin(), wave.end());
    // const auto vmin = *mm.first;
    // const auto vmax = *mm.second;
    // const int nbins = wave.size();
    // const auto binsize = (vmax-vmin)/nbins;
    // Waveform::realseq_t hist(nbins);
    // //std::cerr << "MEDIAN: (" << vmin << "," << vmax <<")/" << binsize << " with " << nbins << std::endl;
    // for (auto val : wave) {
    // 	int bin = int(round((val - vmin)/binsize));
    // 	bin = std::max(0, bin);
    // 	bin = std::min(nbins-1, bin);
    // 	hist[bin] += 1.0;
    // 	//std::cerr << "MEDIAN: " << bin << ":" << val << ", ";
    // }
    // //std::cerr << std::endl;

    // // // debug
    // // for (auto count : hist) {
    // // 	if (!count) { continue; }
    // // 	std::cerr << count << " ";
    // // } std::cerr << std::endl;

    // const int imed = nbins/2;
    // int count = 0;
    // for (int ind=0; ind<nbins; ++ind) {
    // 	count += hist[ind];
    // 	if (count > imed) {
    // 	    float ret = vmin + ind*binsize;
    // 	    //std::cerr << "MEDIAN: [" << count << "] = " << ret << " vmin="<<vmin<<" vmax="<<vmax << " imed="<<imed<<std::endl;
    // 	    return ret;
    // 	}
    // }
    // // can't reach here, return bogus value.
    // return (vmin + vmax)/2.;
}


Waveform::real_t WireCell::Waveform::percentile(Waveform::realseq_t wave, real_t percentage)
{
    std::sort(wave.begin(), wave.end());
    return wave[wave.size() * percentage];
}

Waveform::real_t WireCell::Waveform::percentile_binned(Waveform::realseq_t& wave, real_t percentage){
    const auto mm = std::minmax_element(wave.begin(), wave.end());
    const auto vmin = *mm.first;
    const auto vmax = *mm.second;
    const int nbins = wave.size();
    const auto binsize = (vmax-vmin)/nbins;
    Waveform::realseq_t hist(nbins);
    //std::cerr << "MEDIAN: (" << vmin << "," << vmax <<")/" << binsize << " with " << nbins << std::endl;
    for (auto val : wave) {
	int bin = int(round((val - vmin)/binsize));
	bin = std::max(0, bin);
	bin = std::min(nbins-1, bin);
	hist[bin] += 1.0;
	//std::cerr << "MEDIAN: " << bin << ":" << val << ", ";
    }
    //std::cerr << std::endl;
    
    // // debug
    // for (auto count : hist) {
    // 	if (!count) { continue; }
    // 	std::cerr << count << " ";
    // } std::cerr << std::endl;

    const int imed = wave.size() * percentage;
    int count = 0;
    for (int ind=0; ind<nbins; ++ind) {
	count += hist[ind];
	if (count > imed) {
	    float ret = vmin + ind*binsize;
	    //std::cerr << "MEDIAN: [" << count << "] = " << ret << " vmin="<<vmin<<" vmax="<<vmax << " imed="<<imed<<std::endl;
	    return ret;
	}
    }
    // can't reach here, return bogus value.
    return vmin + (vmax-vmin)*percentage;
}


       
#ifdef MISSING_FFTW_SINGLE_PRECISION

// 15% slowdown casting to double
Waveform::compseq_t WireCell::Waveform::dft(realseq_t wave)
{
    const Eigen::VectorXd vd = Eigen::Map<const Eigen::VectorXf>(wave.data(), wave.size()).cast<double>();
    Eigen::FFT<double> transd;
    const Eigen::VectorXcd retd = transd.fwd(vd);
    const Eigen::VectorXcf ret = retd.cast< std::complex<float> >();
    return compseq_t(ret.data(), ret.data()+ret.size());
}
Waveform::realseq_t WireCell::Waveform::idft(compseq_t spec)
{
    const Eigen::VectorXcd vcd = Eigen::Map<Eigen::VectorXcf>(spec.data(), spec.size()).cast< std::complex<double> >();
    Eigen::FFT<double> transd;
    Eigen::VectorXd retd;
    transd.inv(retd, vcd);
    const Eigen::VectorXf ret = retd.cast<float>();
    return realseq_t(ret.data(), ret.data()+ret.size());
}

#else

Waveform::compseq_t WireCell::Waveform::dft(realseq_t wave)
{
    auto v = Eigen::Map<Eigen::VectorXf>(wave.data(), wave.size());
    Eigen::FFT<Waveform::real_t> trans;
    Eigen::VectorXcf ret = trans.fwd(v);
    return compseq_t(ret.data(), ret.data()+ret.size());
}

Waveform::realseq_t WireCell::Waveform::idft(compseq_t spec)
{
    Eigen::FFT<Waveform::real_t> trans;
    auto v = Eigen::Map<Eigen::VectorXcf>(spec.data(), spec.size());
    Eigen::VectorXf ret;
    trans.inv(ret, v);
    return realseq_t(ret.data(), ret.data()+ret.size());
}

#endif
 



WireCell::Waveform::BinRangeList
WireCell::Waveform::merge(const WireCell::Waveform::BinRangeList& brl)
{
    WireCell::Waveform::BinRangeList tmp(brl.begin(), brl.end());
    WireCell::Waveform::BinRangeList out;
    sort(tmp.begin(), tmp.end());
    Waveform::BinRange last_br = tmp[0];
    out.push_back(last_br);

    for (int ind=1; ind<tmp.size(); ++ind) {
	Waveform::BinRange this_br = tmp[ind];
	if (out.back().second >= this_br.first) {
	  out.back().second = this_br.second;
	  continue;
	}
	out.push_back(this_br);
    }
    return out;
}

	/// Merge two bin range lists, forming a union from any overlapping ranges
WireCell::Waveform::BinRangeList
WireCell::Waveform::merge(const WireCell::Waveform::BinRangeList& br1,
			  const WireCell::Waveform::BinRangeList& br2)
{
    WireCell::Waveform::BinRangeList out;
    out.reserve(br1.size() + br2.size());
    out.insert(out.end(), br1.begin(), br1.end());
    out.insert(out.end(), br2.begin(), br2.end());
    return merge(out);
}




/// Return a new mapping which is the union of all same channel masks.
WireCell::Waveform::ChannelMasks
WireCell::Waveform::merge(const WireCell::Waveform::ChannelMasks& one,
			  const WireCell::Waveform::ChannelMasks& two)
{
    WireCell::Waveform::ChannelMasks out = one;
    for (auto const &it : two) {
	int ch = it.first;
	out[ch] = merge(out[ch], it.second);
    }
    return out;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
