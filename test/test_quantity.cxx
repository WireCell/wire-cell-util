#include "WireCellUtil/Quantity.h"
#include "WireCellUtil/Testing.h"
#include <iostream>

using namespace WireCell;
using namespace std;

int main()
{
    cout << Quantity() << endl;
				   
    Quantity a1(5,1), b1(2,3);
    Quantity a2(5,2), b2(2,.1);

    cout << 1 << a1 << " * " << b1 << " = " << a1*b1 << endl;
    cout << 2 << a1 << " / " << b1 << " = " << a1/b1 << endl;
    cout << 3.0 << a1 << " + " << b1 << " = " << a1+b1 << endl;
    cout << 4 << a1 << " - " << b1 << " = " << a1-b1 << endl;
    cout << 4 << "-" << a1 << " = " << -a1 << endl;

    Assert(a1 < 10);
    Assert(a1 > 4);
    Assert(a1 == 5);
    Assert(a1 == 5.0);
    Assert(a1 == a2);
    Assert(a1 != b1);
    Assert(a1 != 3);

    Assert(b1 < a1);
    Assert(a1 > b1);
    
}
