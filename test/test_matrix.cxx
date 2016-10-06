#include "WireCellUtil/Matrix.h"
#include "WireCellUtil/Waveform.h"

#include "WireCellUtil/Testing.h"
#include "WireCellUtil/MemUsage.h"

#include <iostream>
#include <random>
#include <vector>


using namespace WireCell;
using namespace std;

int main()
{
    MemUsage mu;

    unsigned int seed = 4;	// this is a carefully chosen random number, use only once
    std::default_random_engine re(seed);
    std::normal_distribution<float> dist(1000, 10);
    
    const int nchannels = 3000;
    const int nticks = 10000;
    vector<Waveform::realseq_t> waves(nchannels);
    for (int ichan=0; ichan<nchannels; ++ichan) {
	waves[ichan].resize(nticks,0.0);
	for (int itick=0; itick<nticks; ++itick) {
	    waves[ichan][itick] = dist(re);
	}
    }
    cout << mu("filled vectors") << endl;

    {
	Matrix::real_matrix frame = Matrix::frame_matrix(waves);
	cout << mu("filled frame") << endl;
    }
    cout << mu("out of scope") << endl;    
    cout << "Summary:" << endl;
    cout << mu.summary() << endl;

    return 0;
}
