#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <algorithm>
#include <valarray>
	

using namespace std;
using namespace WireCell;

void test_transform()
{
    const int baseline = 1000;
    const int nticks = 100;
    const int period = 10;

    std::vector<int> wf1(nticks), wf2(nticks);

    for (int ind=0; ind<nticks; ++ind) {
	wf1[ind] = baseline + ind%period;
    }

    transform(wf1.begin(), wf1.end(), wf2.begin(),
	      [](int x)->int{return x - baseline;});

    for (int ind=0; ind<nticks; ++ind) {
	Assert(wf1[ind]-baseline == wf2[ind]);
    }
}

void test_mean_rms()
{
    vector<float> v{1.0,1.0,2.0,3.0,4.0,4.0,4.0,3.0};
    Waveform::signal_t s = Eigen::Map<Waveform::signal_t>(v.data(), v.size());
    auto us = WireCell::Waveform::mean_rms(s);
    cerr << us.first << " +/- " << us.second << endl;
}


void test_fft()
{
    Eigen::VectorXf pi = Eigen::VectorXf::LinSpaced(100,0,7*3.1415);
    Eigen::VectorXf s = pi.array().sin();

    Eigen::VectorXcf f;
    Eigen::FFT<float> fft;

    f = fft.fwd(s);
    cerr << s << endl;
    for (int ind=0; ind<100; ++ind) {
	complex<float> c = f(ind);
	cerr << s(ind) <<"\t"<< c << "\tmag=" << std::abs(c) << "\tphi=" << std::arg(c) << endl;
    }
    cerr << s.size() << " " << f.size() << endl;
}
Waveform::signal_t make_signal(std::vector<Waveform::value_t> vec) {
    return Eigen::Map<Waveform::signal_t>(vec.data(), vec.size());
}

void test_arithmetic()
{
    std::vector<float> v{1.0,1.0,2.0,3.0,4.0,4.0,4.0,3.0};

    Waveform::signal_t a = make_signal(v);
    Waveform::signal_t b = 2*a - 1;
    //cerr << b << endl;
    Waveform::signal_t c = b/(a*a);
    //cerr << c << endl;

    Waveform::fourier_t f = Waveform::fft(c);
    //cerr << f << endl;
}

void test_std_valarray()
{
    std::vector<float> v{1.0,1.0,2.0,3.0,4.0,4.0,4.0,3.0};

    typedef std::valarray<float> vaf;
    vaf a(v.data(), v.size());
    vaf b;
    b = a*2.0;
    b -= 1.0;
    
    vaf c;
    c = b/(a*a);
    //cerr << c << endl;
}

int main(int argc, char* argv[])
{
    test_transform();
    test_mean_rms();
    test_fft();
    test_arithmetic();
    test_std_valarray();
    return 0;
}
