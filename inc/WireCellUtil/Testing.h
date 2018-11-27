#ifndef WIRECELLUTIL_TESTING
#define WIRECELLUTIL_TESTING

// Reject this dumbness https://en.cppreference.com/w/c/error/assert
#undef NDEBUG
#include <boost/assert.hpp>

#define Assert BOOST_ASSERT
#define AssertMsg BOOST_ASSERT_MSG 

// #include <string>
// #include <iostream>

// namespace WireCell {

//     /// Assert ok is true or print msg to cerr
//     template <typename OK>
//     void Assert(const OK& ok, std::string msg="FAIL") {
// 	if (ok) return;
// 	std::cerr << msg << std::endl;
// 	exit(1);
//     }


// }

#endif
