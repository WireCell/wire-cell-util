#include "WireCellUtil/TupleHelpers.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

int main()
{
    using namespace WireCell;

    typedef std::tuple<int,float,double,char,std::string> IFDCS;

    std::vector<std::string> typenames = map_tuple< IFDCS >::type_names();
    for (auto tn : typenames) {
	std::cerr << tn << std::endl;
    }

    IFDCS ifdcs{1,2.2,3.0e-9,'a',"foo"};
    std::vector<boost::any> anyvec = as_any(ifdcs);
    std::cerr << boost::any_cast<int>(anyvec[0]) << std::endl;
    std::cerr << boost::any_cast<std::string>(anyvec[4]) << std::endl;


    typedef typename wrap_tuple<IFDCS>::type ifdcs_queues;
    ifdcs_queues qs;
    std::get<0>(qs).push_back(1);
    std::get<1>(qs).push_back(2.2f);
    std::get<2>(qs).push_back(3.0e-9);
    std::get<3>(qs).push_back('a');
    std::get<4>(qs).push_back(std::string("foo"));
	
    auto any_q = as_any_queue(qs);
    Assert (any_q.size() == 5);
    for (auto q : any_q) {
	Assert(q.size() == 1);
    }

    using boost::any_cast;

    std::cerr << "First element from each queue:\n";
    std::cerr << any_cast<int>(any_q[0][0]) << std::endl;
    std::cerr << any_cast<float>(any_q[1][0]) << std::endl;
    std::cerr << any_cast<double>(any_q[2][0]) << std::endl;
    std::cerr << any_cast<char>(any_q[3][0]) << std::endl;
    std::cerr << any_cast<std::string>(any_q[4][0]) << std::endl;
    

    Assert (any_cast<int>        (any_q[0][0]) == 1);
    //std::cerr << "2.2f - " << any_cast<float>(any_q[1][0]) << " = " << 2.2f - any_cast<float>(any_q[1][0]) << std::endl;
    Assert (any_cast<float>      (any_q[1][0]) == 2.2f);
    Assert (any_cast<double>     (any_q[2][0]) == 3.0e-9);
    Assert (any_cast<char>       (any_q[3][0]) == 'a');
    Assert (any_cast<std::string>(any_q[4][0]) == "foo");

    // for (auto a: as_any(std::get<0>(qs))) {
    // 	std::cerr << boost::any_cast<int>(a) << std::endl;
    // }
    typedef typename wrap_tuple<IFDCS, std::vector >::type ifdcs_vectors;
    ifdcs_vectors vs;
    std::get<0>(vs).push_back(1);
    std::get<1>(vs).push_back(2.2);
    std::get<2>(vs).push_back(3.0e-9);
    std::get<3>(vs).push_back('a');
    std::get<4>(vs).push_back(std::string("foo"));



}
