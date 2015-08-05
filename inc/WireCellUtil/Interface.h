#ifndef WIRECELL_INTERFACE
#define WIRECELL_INTERFACE

#include <memory>

namespace WireCell {

    class Interface {
    public:
	// Interfaces are accessed by non-const shared pointers.
	typedef std::shared_ptr<Interface> pointer;

	virtual ~Interface();
    };
}

#endif

