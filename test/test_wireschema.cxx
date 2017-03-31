#include "WireCellUtil/WireSchema.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
using namespace std;
using namespace WireCell;

void test_read(const char* filename)
{
    auto store = WireSchema::load(filename);

    cerr << "Got " << store.anodes.size() << " anodes\n";
    cerr << "Got " << store.faces.size() << " faces\n";
    cerr << "Got " << store.planes.size() << " planes\n";
    cerr << "Got " << store.wires.size() << " wires\n";

    Assert(store.wires.size() > 0);
}


int main(int argc, char *argv[])
{

    if (argc > 1) {
        test_read(argv[1]);
    }

    return 0;

}
