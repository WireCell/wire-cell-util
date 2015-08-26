#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Fanout.h"

#include <iostream>
#include <vector>

struct Counter {
    int count;
    Counter(int c=0) : count(c) {}
    int operator()(){
	return count++;
    }
};


using namespace std;
using namespace WireCell;

void test_plug_and_play()
{
    Counter counter;
    Fanout<int> fanout;
    fanout.connect(counter);

    int addresses[] = {1,2,3,0,0,0,2,4,4,4,4,4,1,1,2,1,1,-1};
    int want_val[]  = {0,1,2,3,4,5,2,0,1,2,3,4,1,2,3,3,4,-1};

    // pre-register 
    fanout.address(4);

    for (int ind=0; addresses[ind] >= 0; ++ind) {
	int addr = addresses[ind];
	if (addr < 0) { break; }
	int want = want_val[ind];
	int got = fanout(addr);
	cerr << "addr=" << addr << " got:" << got << " want:" << want << endl;
	Assert(want == got);
    }

}

template<typename Collection>
struct CollectionCombiner {
    typedef Collection result_type;
    typedef typename Collection::value_type value_type;

    template<typename InputIterator>
    result_type operator()(InputIterator first, InputIterator last) const {
	result_type ret;
	while (first != last) {
	    ret.push_back(*first);
	    ++first;
	}
	return ret;
    }
};


int test_fanin()
{
    Counter c1(10),c2(20),c3(30);

    // Fan-in concept is inherent in boost::signals2, but does require
    // some "combiner" to enact whatever fan-in policy is desired.
    // This one synchronizes input into a collection.
    boost::signals2::signal< int (), CollectionCombiner< std::vector<int> > > sig;
    sig.connect(c1);
    sig.connect(c2);
    sig.connect(c3);

    for (int ind=0; ind<10; ++ind) {
	auto vec = sig();
	cerr << ind;
	for (auto x: vec) {
	    cerr << " " << x;
	}
	cerr << endl;
    }
    
}

int main()
{
    test_plug_and_play();
    test_fanin();
    return 0;
}
