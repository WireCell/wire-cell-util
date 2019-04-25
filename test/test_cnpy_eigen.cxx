#include "WireCellUtil/cnpy.h"

#include <Eigen/Core>

typedef Eigen::Array<short, Eigen::Dynamic, Eigen::Dynamic> ArrayXXs;

const int Nchanc = 480;
//const int Nchani = 800;
const int Ntick = 2000;

int main(int argc, char* argv[])
{
    std::string name = argv[0];
    name += ".npz";

    //ArrayXXs a = ArrayXXs::Random(Nchanc,Ntick);
    ArrayXXs a = ArrayXXs::Zero(Nchanc,Ntick);
    a(0,1000) = 1000;
    a(400,0) = 400;
    const short* data = a.data();
    cnpy::npz_save<short>(name.c_str(), "a", data, {Ntick, Nchanc}, "w");
    /*
      >>> import numpy
      >>> f = numpy.load("eigentest.npz")
      >>> a = f['a']
      >>> print a.shape, a[0,400], a[1000,0]
      (2000, 480) 400 1000
     */
    return 0;
}



