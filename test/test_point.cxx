#include "WireCellUtil/Point.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
using namespace std;
using namespace WireCell;

int main()
{
    Point p1(1,2,3);
    Assert (p1.x == 1 && p1.y == 2 && p1.z == 3);

    Point p2 = p1;

    Assert(p1 == p2);

    Point p3(p1);
    Assert(p1 == p3);

    PointF pf(p1);
    Assert(Point(pf) == p1);
    Assert(pf == PointF(p1));

    Point ps = p1.sum(p2);
    Assert (ps.x == 2);
	
    Assert (p1.norm().magnitude() == 1.0);
}
