#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Testing.h"

#include <string>
#include <iostream>
#include <cmath>
#include <cstdlib>              // for getenv

using namespace std;
using namespace WireCell;

// from Jsonnet docs
const std::string give1 = R"(
{
    person1: {
        name: "Alice",
        welcome: "Hello " + self.name + "!",
    },
    person2: self.person1 { name: "Bob" },
}
)";
const std::string want1 = R"({
   "person1": {
      "name": "Alice",
      "welcome": "Hello Alice!"
   },
   "person2": {
      "name": "Bob",
      "welcome": "Hello Bob!"
   }
}
)";

int main()
{
    string got1 = Persist::evaluate_jsonnet(give1);
    // cerr << "------give:\n";
    // cerr << give1 << endl;
    // cerr << "------got:\n";
    // cerr << got1 << endl;
    // cerr << "------want:\n";
    // cerr << want1 << endl;
    // cerr << "------\n";
    Assert(got1 == want1);

    if (!std::getenv("JSONNET_PATH")) {
        cerr << "test_jsonnet requires setting JSONNET_PATH to point to where 'wirecell.jsonnet' exists\n";
        return 0;
    }
    string text = Persist::evaluate_jsonnet("local wc = import \"wirecell.jsonnet\"; [ wc.pi ]");
    //cerr << text << endl;
    auto res = Persist::loads(text);

    const double mypi = 2*std::acos(0.0);
    const double jspi = res[0].asDouble();
    Assert (std::abs(jspi - mypi) < 1.0e-16);
    

    return 0;
}
