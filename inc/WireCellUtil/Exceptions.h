/** 
    Wire Cell code may throw exceptions.  

    The core Wire Cell code should only throw WireCell::Exception (or
    a subclass).

    Boost exceptions are used to allow the programmer to provide
    dynamic information about an error and to collect file name and
    line number.

    Exceptions should be thrown something like:

    if (omg) {
        THROW(ValueError() << errmsg{"I didn't expect that"});
    }
 */

#ifndef WIRECELL_EXCEPTIONS
#define WIRECELL_EXCEPTIONS

#include <boost/exception/all.hpp>
#include <exception>
#include <string>

#define THROW(e) BOOST_THROW_EXCEPTION(e)
#define errstr(e) boost::diagnostic_information(e)

namespace WireCell {

    /// The base wire cell exception.
    struct Exception : virtual public std::exception , virtual boost::exception {
        char const *what() const throw() { return diagnostic_information_what(*this); }
    };


    /// Thrown when a wrong value has been encountered.
    struct ValueError : virtual public Exception { } ;

    /// Thrown when a wrong index is used.
    struct IndexError : virtual public Exception { } ;

    /// Thrown when a wrong key or has been encountered.
    struct KeyError : virtual public Exception { } ;

    /// Thrown when an error involving accessing input or output has occurred.
    class IOError : virtual public Exception { } ;

    /// Thrown when an error occurs during the data processing
    class RuntimeError : virtual public Exception { } ;

    /// Thrown when an assertion fails
    class AssertionError : virtual public Exception { } ;

    typedef boost::error_info<struct tag_errmsg, std::string> errmsg;

}
#endif
