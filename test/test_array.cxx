#include "WireCellUtil/Array.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

using namespace std;
using namespace WireCell;
using namespace WireCell::Array;


array_xxf my_great_array(ExecMon& em, int nrows, int ncols)
{
    std::stringstream ss;
    ss << "array(" << nrows << "," << ncols<<")";
    em(ss.str() + ": constructing");
    array_xxf arr = Eigen::ArrayXXf::Random(nrows, ncols); // 197 ms opt, 646 ms debug.
    em(ss.str() + ": constructed");

    return arr;
}

array_xxf dup(const array_xxf& arr)
{
    return arr;
}

void test_copy(ExecMon& em)
{
    const int nrows = 3000;
    const int ncols = 10000;
    {
	array_xxf arr = my_great_array(em, nrows, ncols);
	em("array by value: returned");

	// This loop over 100 copies takes 19.513 s (debug), 1.715 s (opt)
	for (int ind=0; ind<100; ++ind) {
	    array_xxf tmp = dup(arr);
	}
	em("array by value: 100 dups");

    }
    em("array by value: left scope");
}

void test_return(ExecMon& em)
{
    const int nrows = 3000;
    const int ncols = 10000;
    {
	auto arr = my_great_array(em, nrows, ncols);
	em("ret: to auto");
	Assert(arr.rows() == nrows);
    }
    em("ret: out of scope");
    
}


void test_dft(ExecMon& em)
{
    const int nrows = 300;
    const int ncols = 1000;

    const int nrounds = 100;

    auto arr = my_great_array(em, nrows, ncols);
    em("dft: make array");
    for (int count = 0; count < nrounds; ++count) {

	auto spec = dft(arr);
        auto orig = idft(spec);

    }
    em("dft with floats");
#ifdef WCT_HACK_FOR_FFTW_NO_SP
    for (int count = 0; count < nrounds; ++count) {
        auto spec = dftd(arr);
        auto orig = idftd(spec);
    }
    em("dft up/down cast through doubles");
#endif
}

void test_deconv(ExecMon& em)
{
    const int nrows = 300;
    const int ncols = 1000;

    em("deconv: start");
    auto arr = my_great_array(em, nrows, ncols);
    em("deconv: got array");
    array_xxc filt = Eigen::ArrayXXcf::Zero(nrows, ncols) + 1.0;
    em("deconv: got filter");
    auto deco = deconv(arr, filt);
    em("deconv: done"); // 38ms opt, 274 debug
    
    array_xxf diff = arr - deco;
    em("deconv: diff");

    double norm = diff.matrix().norm();
    em("deconv: norm");
    cerr << "got norm of diff " << norm << endl;
    Assert(norm < 0.001);
}

int main()
{
    WireCell::ExecMon em;

    test_copy(em);
    test_return(em);
    test_dft(em);
    test_deconv(em);

    em("the end");
    cerr << em.summary() << endl;
    return 0;
}
