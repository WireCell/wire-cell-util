#include "WireCellUtil/MemUsage.h"

#include <boost/align/aligned_allocator.hpp>
#include <iostream>
#include <vector>

typedef std::vector<float> nominal_vector;
typedef std::vector<float, boost::alignment::aligned_allocator<float, 4096> > aligned_vector;


template<typename Vec>
void make(std::vector<Vec*>& vec, bool block)
{
    const size_t nchans = 2400+2400+3456;
    const size_t nticks = 9600;
    for (size_t ind=0; ind<nchans; ++ind) {
	if (block) {
	    auto v = new Vec(nticks);
	    vec.push_back(v);
	    continue;
	}
	auto v = new Vec;
	for (size_t itick=0; itick < nticks; ++itick) {
	    v->push_back(0.0);
	}
	vec.push_back(v);
    }
}
template<typename Vec>
void clear(std::vector<Vec*>& vec)
{
    for (auto& v : vec) {
	delete v;
    }
    vec.clear();
}

template<typename MU>
void test_naan(MU& mu, bool block)
{
    std::string what = "push_back";
    if (block) {
	what = "blocked";
    }

    std::vector<nominal_vector*> nominal;
    make(nominal, block);
    mu("make nominal " + what);

    std::vector<aligned_vector*> aligned;
    make(aligned, block);
    mu("make aligned " + what);

    clear(aligned);
    mu("clear aligned " + what);

    clear(nominal);
    mu("clear nominal " + what);
}
template<typename MU>
void test_anna(MU& mu, bool block)
{
    std::string what = "push_back";
    if (block) {
	what = "blocked";
    }

    std::vector<aligned_vector*> aligned;
    make(aligned, block);
    mu("make aligned " + what);

    std::vector<nominal_vector*> nominal;
    make(nominal, block);
    mu("make nominal " + what);

    clear(nominal);
    mu("clear nominal " + what);

    clear(aligned);
    mu("clear aligned " + what);
}
template<typename MU>
void test_nnnn(MU& mu, bool block)
{
    std::string what = "push_back";
    if (block) {
	what = "blocked";
    }

    std::vector<nominal_vector*> one;
    make(one, block);
    mu("make nominal1 " + what);

    std::vector<nominal_vector*> two;
    make(two, block);
    mu("make nominal2 " + what);

    clear(two);
    mu("clear nominal2 " + what);

    clear(one);
    mu("clear nominal1 " + what);
}
template<typename MU>
void test_aaaa(MU& mu, bool block)
{
    std::string what = "push_back";
    if (block) {
	what = "blocked";
    }

    std::vector<aligned_vector*> one;
    make(one, block);
    mu("make aligned1 " + what);

    std::vector<aligned_vector*> two;
    make(two, block);
    mu("make aligned2 " + what);

    clear(two);
    mu("clear aligned2 " + what);

    clear(one);
    mu("clear aligned1 " + what);
}

struct DummyMu {
    void operator()(const std::string& ignored) { };
};

int main(int argc, char* argv[])
{

    WireCell::MemUsage mu;
    mu("start");


    std::string what = "naan";
    if (argc > 1) {
	what = argv[1];
    }
    bool blocked = true;
    if (argc > 2) {
	std::string want = argv[2];
	if (want != "blocked") {
	    blocked = false;
	}
    }
    int tries = 1;
    if (argc > 3) {
	tries = atoi(argv[3]);
    }


    for (int itry=0; itry<tries; ++itry) {
	mu("start 'event'");
	if (what == "naan") {
	    test_naan(mu, blocked);
	}
	if (what == "anna") {
	    test_anna(mu, blocked);
	}
	if (what == "nnnn") {
	    test_nnnn(mu, blocked);
	}
	if (what == "aaaa") {
	    test_aaaa(mu, blocked);
	}
	mu("end 'event'");
    }

    mu("done");
    std::cout << mu.summary() << std::endl;

    return 0;
}

