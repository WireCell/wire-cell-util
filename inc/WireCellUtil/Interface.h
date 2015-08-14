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

    // crtp
    template<class Type>
    class TypedInterface : virtual public Interface{
    public:
	// provide a pointer typed to the interface type.  Subclass
	// should likely NOT override this.
	typedef std::shared_ptr<Type> pointer;

	virtual ~TypedInterface() {};
    };
}

#endif

