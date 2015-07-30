// boost iterator facade look-ahead test

#include "WireCellUtil/GeneratorIter.h"

#include <iostream>
using namespace std;
using namespace WireCell;


template<typename Container>
struct EveryOther {
    typedef typename Container::value_type value_type;
    
    typename Container::const_iterator end, it, other;

    EveryOther(const Container& seq) : end(seq.end()), it(seq.begin()), other(seq.begin()) {
	if (other != end) ++other;
	if (other != end) ++other;
	cerr << "Creating EveryOther starting at " << *it << " and " << *other << endl;
    }
    value_type operator()() {
	value_type ret;
	if (*this) {
	    ret = *it + *other;
	    ++it;
	    ++other;
	}
	cerr << "operator() returns " << ret << endl;
	return ret;
    }
    operator bool() const {
	bool exhausted = false;
	if (end-it < 2) {
	    exhausted = true;
	}
	cerr << "Are we exhausted? " << exhausted << endl;
	return !exhausted;
    }
    bool operator==(const EveryOther& rhs) const {
	if (this == &rhs) return true;
	return it == rhs.it;
    }
};

#include <vector>

int main()
{
    typedef std::vector<int> IntVector;
    IntVector vi;
    vi.push_back(0);
    vi.push_back(1);
    vi.push_back(2);
    vi.push_back(3);
    vi.push_back(4);
    vi.push_back(5);
    vi.push_back(6);
    

    typedef EveryOther<IntVector> EOIV;
    EOIV eo(vi);
    GeneratorIter<EOIV, int> gi(eo);
    for (int ind=0; ind<10; ++ind) {
	if (!gi) { break; }
	cout <<"In loop: #" << ind << " value:" << *gi << endl;
	++gi;
    }
}
