#include "WireCellUtil/Matrix.h"
#include "WireCellUtil/Waveform.h"

#include "WireCellUtil/Testing.h"
#include "WireCellUtil/ExecMon.h"

#include <iostream>
#include <random>
#include <vector>


using namespace WireCell;
using namespace std;


bool check_same(Matrix::real_matrix m1, Matrix::real_matrix m2, double epsilon = 0.00001)
{
    if (m1->rows() != m2->rows()) {
	cerr << "unequal rows: " << m1->rows() << " != " << m2->rows() << endl;
	return false;
    }
    if (m1->cols() != m2->cols()) {
	cerr << "unequal cols: " << m1->cols() << " != " << m2->cols() << endl;
	return false;
    }

    int ndiff = 0;
    const int nrows = m1->rows();
    const int ncols = m1->cols();
    for (int irow=0; irow<nrows; ++irow) {
	auto r1 = m1->row(irow);
	auto r2 = m2->row(irow);
	for (int icol=0; icol<ncols; ++icol) {
	    double diff = r1(icol) - r2(icol);
	    double avg = 0.5*(r1(icol) + r2(icol));
	    double rel = diff/avg;
	    if (std::abs(rel) > epsilon) {
		cerr << "delta("<<irow<<","<<icol<<") = " << r1(icol) <<" - "<<r2(icol) << " = " << rel << " < " << epsilon << endl;
		++ndiff;

	    }
	}
    }
    return ndiff == 0;
}

int main()
{
    ExecMon em;

    unsigned int seed = 4;	// this is a carefully chosen random number, use only once
    std::default_random_engine re(seed);
    std::normal_distribution<float> dist(1000, 10);
    
    // at factor==100 it takes 16.5 seconds unoptimized, 2.5 seconds
    //optimized for both directions of 2D FFT and 5 seconds to perform
    //some trivial loop over entire matrix. 


    const int factor = 100;
    const int nchannels = 30*factor;
    const int nticks = 100*factor;

    vector<Waveform::realseq_t> waves(nchannels);
    for (int ichan=0; ichan<nchannels; ++ichan) {
	waves[ichan].resize(nticks,0.0);
	for (int itick=0; itick<nticks; ++itick) {
	    waves[ichan][itick] = dist(re);
	}
    }
    cout << em("filled vectors") << endl;

    Matrix::real_matrix frame = Matrix::frame_matrix(waves);
    cout << em("filled matrix") << endl;

    waves.clear();
    cout << em("cleared vectors") << endl;

    Matrix::complex_matrix pfspec = Matrix::dft(frame);
    cout << em("after fwd DFT") << endl;    

    Matrix::real_matrix frame2 = Matrix::idft(pfspec);
    cout << em("after inv DFT") << endl;

    pfspec = nullptr;
    cout << em("cleared DFT") << endl;

    Assert (check_same(frame, frame2));
    cout << em("after test same") << endl;

    frame = nullptr;
    cout << em("cleared original matrix") << endl;

    frame2 = nullptr;
    cout << em("cleared final matrix") << endl;    


    cout << "Summary:" << endl;
    cout << em.summary() << endl;

    return 0;
}
