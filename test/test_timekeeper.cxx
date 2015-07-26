/** Test the WireCellUtil/Testing helpers.
 */

#include "WireCellUtil/Testing.h"
#include "WireCellUtil/TimeKeeper.h"

#include <unistd.h> // fixme: maybe a more modern sleep() is preferable?

using namespace WireCell;
using namespace std;

int main(int argc, char** argv)
{
    TimeKeeper tk("test_testing");
    cout << tk("sleeping") << endl;;
    sleep(1);
    cout << tk("awake") << endl;

    TimeKeeper::deltat dt = tk.last_duration();
    Assert(dt.seconds() == 1, "Bad sleep.");

    cout << "TimeKeeper summary: " << endl;
    cout << tk.summary() << endl;
}
