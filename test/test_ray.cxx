#include "WireCellUtil/Point.h"
#include "WireCellUtil/Testing.h"
#include <iostream>
using namespace std;
using namespace WireCell;

int main()
{

    const Ray r1(Point(-1,-1,-1), Point(1,1,1));
    const Ray r2(Point(0,0,0), Point(1,1,1));
    cerr << "Some rays:" << r1 << ", " << r2 << endl;

    const Ray& rr1 = r1;
    const Ray& rr2 = r2;
    cerr << "Some ray refs:" << rr1 << ", " << rr2 << endl;
}
