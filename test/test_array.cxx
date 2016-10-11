#include "WireCellUtil/Array.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

using namespace std;
using namespace WireCell;
using namespace WireCell::Array;


const_shared_array_xxf my_great_array(int nrows, int ncols)
{
    shared_array_xxf arr = std::make_shared<array_xxf>(nrows, ncols);
    (*arr) = Eigen::ArrayXXf::Random(nrows, ncols);
    return arr;
}

void test_return(ExecMon& em)
{
    const int nrows = 3000;
    const int ncols = 10000;
    {
	auto arr = my_great_array(nrows, ncols);
	em("ret: to auto");
	Assert(arr->rows() == nrows);
    }
    em("ret: out of scope");
    {
	const_shared_array_xxf arr = my_great_array(nrows, ncols);
	em("ret: to explicit");
	Assert(arr->rows() == nrows);
	arr = nullptr;
	em("ret: nullify");
    }
    
}


void test_dft(ExecMon& em)
{
    const int nrows = 300;
    const int ncols = 1000;

    auto arr = my_great_array(nrows, ncols);
    em("dft: make array");
    {
	auto spec = dft(*arr);
	em("dft: to auto");
	Assert(arr->rows() == nrows);
    }
    em("dft to auto out of scope");
    {
	const_shared_array_xxc spec = dft(*arr);
	em("dft: to explicit");
	Assert(arr->rows() == nrows);
	spec = nullptr;
	em("dft: nullify dft output");
    }
    arr = nullptr;
    em("dft: nullify original array");
}

void test_deconv(ExecMon& em)
{
    const int nrows = 300;
    const int ncols = 1000;

    em("deconv: start");
    auto arr = my_great_array(nrows, ncols);
    em("deconv: got array");
    array_xxc filt = Eigen::ArrayXXcf::Zero(nrows, ncols) + 1.0;
    em("deconv: got filter");
    auto deco = deconv(*arr, filt);
    em("deconv: done");
    
    array_xxf diff = arr->array() - deco->array();
    em("deconv: diff");

    double norm = diff.matrix().norm();
    em("deconv: norm");
    cerr << "got norm of diff " << norm << endl;
    Assert(norm < 0.001);
}

int main()
{
    WireCell::ExecMon em;

    test_return(em);
    test_dft(em);
    test_deconv(em);

    em("the end");
    cerr << em.summary() << endl;
    return 0;
}
