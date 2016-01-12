/** TupleHelpers help tuples help her make a great meal.
 *
 * These helpers provide template gymnastics to hide some complicated
 * code to access tuples more easily.
 *
 * The doc strings on each helper assume an example tuple type like:
 *
 *     typedef std::tuple<int,float,double,char,std::string> IFDCS;
 *
 * For working examples, see test_tuple.cxx.
 */

#ifndef WIRECELLUTIL_TUPLEHELPERS
#define WIRECELLUTIL_TUPLEHELPERS

#include <boost/any.hpp>

#include <tuple>
#include <vector>
#include <deque>
#include <string>
#include <typeinfo>
#include <type_traits> 
#include <utility>


namespace WireCell {


    // much inspiration from:
    // https://www.preney.ca/paul/archives/486
    // http://loungecpp.wikidot.com/tips-and-tricks:indices
    //
    // fixme: the patterns of each helper differ as different tips
    // were followed.  It would be good to clean this up.

    /** Compile-time mapping of methods on tuple element types
     * 
     *     std::vector<std::string> typenames = map_tuple< IFDCS >::type_names();
     */
    template <typename Tuple>
    struct map_tuple;
    template<
	template <typename...> class T,
	typename Type,
	typename... Types
	>
    struct map_tuple< T<Type, Types...> >
    {
	typedef T<Type, Types...> tuple_type;

	static std::vector<std::string> type_names() {
	    std::vector<std::string> ret = map_tuple< T<Types...> >::type_names();
	    ret.insert(ret.begin(), std::string(typeid(Type).name()));
	    return ret;
	}
    };
    template<
	template <typename...> class T,
	typename Type
	>
    struct map_tuple< T<Type> >
    {
	static std::vector<std::string> type_names() {
	    return std::vector<std::string>{std::string(typeid(Type).name())};
	}
    };

	

    /** Define a tuple-of-container-of-elements type from a tuple-of-elements.
     *
     *     typedef typename wrap_tuple<IFDCS>::type ifdcs_queues;
     *     ifdcs_queues my_tuple_of_queues;
     */
    template <
	typename Tuple,
	template <typename...> class Container = std::deque
	>
    struct wrap_tuple;
    template<
	template <typename...> class T,
	template <typename...> class Container,
	typename... Types
	>
    struct wrap_tuple< T<Types...>, Container >
    {
	typedef std::tuple<Container<Types>... > type;
    };


    

    // http://loungecpp.wikidot.com/tips-and-tricks:indices
    // http://stackoverflow.com/questions/31463388/can-someone-please-explain-the-indices-trick
    template <std::size_t... Indices>
    struct indices {};

    template <std::size_t N, std::size_t... Is>
    struct build_indices : build_indices<N-1, N-1, Is...> {};

    template <std::size_t... Is>
    struct build_indices<0, Is...> : indices<Is...> {};
	
    template<typename Tuple, std::size_t... Indices>
    std::vector<boost::any> as_any_detail(const Tuple& t, indices<Indices...>) {
     	return { boost::any(std::get<Indices>(t))... };
    }

    /** Convert a tuple of types to a vector of boost::any.
     *
     *     IFDCS ifdcs{1,2.2,3.0e-9,'a',"foo"};
     *     std::vector<boost::any> anyvec = as_any(ifdcs);
     */
    template <typename Tuple>
    std::vector<boost::any> as_any(const Tuple&t) {
    	return as_any_detail(t, build_indices<std::tuple_size<Tuple>::value>{});
    }


    template<typename Element>
    std::deque<boost::any> as_any_queue_convert(const std::deque<Element>& in) {
	return std::deque<boost::any>(in.begin(), in.end());
    }

    template<typename Tuple, std::size_t... Indices>
    std::vector< std::deque<boost::any> > as_any_queue_detail(const Tuple& t, indices<Indices...>) {
     	return { as_any_queue_convert(std::get<Indices>(t))... };
    }

    /** Convert a tuple of queues of types to a vector of queues of boost::any.
     *
     *     typedef typename wrap_tuple<IFDCS>::type ifdcs_queues;
     *     ifdcs_queues my_tuple_of_queues;
     *     auto any_q = as_any_queue(my_tuple_of_queues);
     *     cout << "First element from each queue:\n";
     *     cout << any_cast<int>(any_q[0][0]) << endl;
     *     cout << any_cast<float>(any_q[1][0]) << endl;
     *     cout << any_cast<double>(any_q[2][0]) << endl;
     *     cout << any_cast<char>(any_q[3][0]) << endl;
     *     cout << any_cast<std::string>(any_q[4][0]) << endl;
     */
    template <typename Tuple>
    std::vector< std::deque<boost::any> > as_any_queue(const Tuple&t) {
    	return as_any_queue_detail(t, build_indices<std::tuple_size<Tuple>::value>{});
    }

}


#endif
