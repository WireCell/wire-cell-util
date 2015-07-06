#include "WireCellUtil/Testing.h"
#include "WireCellUtil/IndexedSet.h"

using namespace WireCell;



int main()
{
    IndexedSet<int> isi;
    Assert (isi(42) == 0, "Failed to index");
    Assert (isi(69) == 1, "Failed to index");
    Assert (isi(42) == 0, "Failed to index");
    Assert (isi(0) == 2, "Failed to index");
    Assert (isi(0) == 2, "Failed to index");
    Assert (isi(69) == 1, "Failed to index");

    Assert(isi.collection.size() == 3, "Wrong number of stuff in the collection");

    return 0;
}
