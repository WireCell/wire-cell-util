#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Exceptions.h"

#include <string>
#include <iostream>
#include <cmath>
#include <cstdlib>              // for getenv

using namespace std;
using namespace WireCell;

// from Jsonnet docs
const std::string give1 = R"(
std.mergePatch({
    person1: {
        name: "Alice",
        welcome: "Hello " + self.name + "!",
    },
    person2: self.person1 { name: "Bob" },
    person3: self.person1 { name: std.extVar("person") },
}, std.extVar("override"))
)";
const std::string want1 = R"({
   "person1": {
      "name": "Alice",
      "welcome": "Hello Alice!"
   },
   "person2": {
      "name": "Bob",
      "welcome": "Hello Bob!"
   },
   "person3": {
      "name": "Malory",
      "welcome": "Hello Malory!"
   },
   "person4": {
      "name": "Surley",
      "welcome": "Don't call my Shirley"
   }
}
)";
//' <-- make emacs c++ mode happy about matched quotes

int main()
{
    Persist::externalvars_t extravars{ {"person", "Malory"} };
    Persist::externalvars_t extracode{ {"override",
                "{person4: {name: \"Surley\", welcome:\"Don't call my Shirley\"}}"} };

    string got1 = Persist::evaluate_jsonnet_text(give1, extravars, extracode);
    cerr << "------give:\n";
    cerr << give1 << endl;
    cerr << "------got:\n";
    cerr << got1 << endl;
    cerr << "------want:\n";
    cerr << want1 << endl;
    cerr << "------\n";
    Assert(got1 == want1);

    try {
        cerr << "There should be errors following:\n";
        string what = Persist::evaluate_jsonnet_text("[ std.extVar(\"doesnoteexists\") ]");
        cerr << what << endl;
    }
    catch (Exception& e) {
        cerr << "Properly caught reference to nonexistent extVar\n";
    }


    if (!std::getenv("WIRECELL_PATH")) {
        cerr << "test_jsonnet requires setting WIRECELL_PATH to point to where 'wirecell.jsonnet' exists\n";
        return 0;
    }
    string text = Persist::evaluate_jsonnet_text("local wc = import \"wirecell.jsonnet\"; [ wc.pi ]");
    //cerr << text << endl;
    auto res = Persist::loads(text);

    const double mypi = 2*std::acos(0.0);
    const double jspi = res[0].asDouble();
    Assert (std::abs(jspi - mypi) < 1.0e-16);

    {
        cerr << "Exercise Parser class:\n";
        Persist::Parser p({}, extravars, extracode);
        auto j = p.loads(give1);
        cerr << j << endl;
    }

    return 0;
}
