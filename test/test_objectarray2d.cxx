#include "WireCellUtil/ObjectArray2d.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

using namespace WireCell;
using namespace std;

int main()
{
    ObjectArray2d<Point> parr(3,3); // 3x3 array of points

    int count = 0;
    for (int i=0; i<3; ++i) {
        for (int j=0; j<3; ++j) {
            ++count;
            parr(i,j).set(i,j,count);
        }
    }

    for (const auto& p : parr) {
        cout << p << endl;
    }

    Assert(parr(0,0));
    parr(0,0).invalidate();
    Assert(!parr(0,0));
    return 0;
}
