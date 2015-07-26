/** Test the WireCellUtil/Testing helpers.
 */

#include "WireCellUtil/Testing.h"

int main(int argc, char** argv)
{
    WireCell::Assert(argc == 1, "In general, tests should not be called with arguments.");

    return 0;
}
