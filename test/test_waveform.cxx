#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <algorithm>
#include <complex>

	

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
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
    Waveform::realseq_t s = Waveform::std2eig(v);
    auto us = WireCell::Waveform::mean_rms(s);
    auto m = Waveform::median(s);

    cerr << us.first << " +/- " << us.second << " med=" << m << endl;
    cerr << s << endl;
    cerr << s-m << endl;
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
	Waveform::complex_t c = f(ind);
	cerr << s(ind) <<"\t"<< c << "\tmag=" << std::abs(c) << "\tphi=" << std::arg(c) << endl;
    }
    cerr << s.size() << " " << f.size() << endl;
}

void test_arithmetic()
{
    std::vector<float> v{1.0,1.0,2.0,3.0,4.0,4.0,4.0,3.0};
    Waveform::realseq_t a = Waveform::std2eig(v);
    std::vector<float> v2 = Waveform::eig2std(a);
    Assert(v == v2);

    Waveform::realseq_t b = 2*a - 1;
    //cerr << b << endl;
    Waveform::realseq_t c = b/(a*a);
    //cerr << c << endl;

    Waveform::compseq_t f = Waveform::dft(c);
    //cerr << f << endl;
}


void test_complex()
{
    std::vector<Waveform::complex_t> cv{{1.1,2.2},{-3.3,4.4},{0,0},{1,0},{0,1},{-1,0},{0,-1}};
    Waveform::compseq_t cw = Waveform::std2eig(cv);
    cerr << "complex:\n" << cw << endl;

    Eigen::ArrayXXf table(cv.size(), 4);
    table.col(0) = Waveform::real(cw);
    table.col(1) = Waveform::imag(cw);
    table.col(2) = Waveform::magnitude(cw);
    table.col(3) = Waveform::phase(cw);

    cerr << "   real    imag     mag   phase" << endl;
    cerr << table << endl;
}

int one_mask(const Waveform::Domain& sd, int nsamps)
{
    std::vector<Waveform::complex_t> cv;
    for (int ind=0; ind<nsamps; ++ind) {
	cv.push_back(Waveform::complex_t(ind+1,ind+1));
    }
    cerr << "mask with " << cv.size() << " elements" << endl;

    Waveform::compseq_t cw = Waveform::std2eig(cv);
    cerr << "before mask:\n"<<cw<<endl;
    Waveform::mask(cw, Waveform::Domain(0,1), sd);
    cerr << "after mask:\n"<<cw<<endl;

    int nonzero=0;
    for (int ind=0; ind<nsamps; ++ind) {
	if (std::abs(cw(ind)) > 0.0) {
	    ++nonzero;
	}
    }
    return nonzero;
}

void test_mask()
{
    const int nsamp=10;
    Assert(0 == one_mask(Waveform::Domain(0.0,1.0), nsamp));
    Assert(2 == one_mask(Waveform::Domain(0.1,0.9), nsamp));
    Assert(2 == one_mask(Waveform::Domain(0.0001,0.9999), nsamp));
}


int main(int argc, char* argv[])
{
    test_transform();
    test_fft();
    test_arithmetic();
    test_complex();
    test_mask();
    test_mean_rms();

    cerr << "bye." << endl;
    return 0;
}
