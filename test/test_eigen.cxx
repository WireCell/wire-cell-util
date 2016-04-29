#include "WireCellUtil/Testing.h"

#include <Eigen/Core>

#include <cmath>
#include <vector>
#include <iostream>

Eigen::ArrayXf vec2arr(const std::vector<float>& v)
{
    Eigen::ArrayXf ret(v.size());
    for (int ind=0; ind<v.size(); ++ind) {
	ret(ind) = v[ind];
    }
    return ret;
}

int main()
{
    using namespace std;
    using namespace Eigen;

    std::vector<float> v{1.0,1.0,2.0,3.0,4.0,4.0,4.0,3.0};
    ArrayXf ar1 = vec2arr(v);	// copy okay

    /// You must specify storage size at construction
    //ArrayXf ar2;
    ArrayXf ar2(v.size());

    /// this doesn't work:
    //ar2 << v;
    /// but literal comma list does:
    ar2 << 1.0,1.0,2.0,3.0,4.0,4.0,4.0,3.0;

    /// or, map the data
    ArrayXf ar3 = Map<ArrayXf>(v.data(), v.size());
    
    ArrayXXf table(ar1.size(), 3);
    table.col(0) = ar1;
    table.col(1) = ar2;
    table.col(2) = ar3;
    cerr << table <<endl;
    VectorXf v1 = ar1.matrix();

    for (int ind=0; ind < v.size(); ++ind) {
	Assert(v[ind] == ar1(ind));
	Assert(v[ind] == ar2(ind));
	Assert(v[ind] == ar3(ind));
	Assert(v[ind] == v1(ind));
    }

    cerr << ar1.size() << " " << ar1.sum() << " " << ar1.prod() << " " << v1.norm() << " " << v1.squaredNorm() << endl;
    int n = v1.size();
    float sigma = sqrt(v1.squaredNorm()/n - ar1.mean()*ar1.mean());
    cerr << ar1.mean() << " +/- " << sigma << endl;

    ArrayXf::Index maxI=-1, minI=-1;
    float minV = ar1.minCoeff(&minI);
    float maxV = ar1.maxCoeff(&maxI);

    Assert(minI == 0);
    Assert(maxI == 4);

    cerr << minV << "@" << minI 
	 << " < "
	 << maxV << "@" << maxI
	 << endl;

    return 0;
}
