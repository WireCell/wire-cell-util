#ifndef WIRECELL_INTERFACE
#define WIRECELL_INTERFACE

#include <memory> 		// shared_ptr

// Call this macro in your IMyInterface.h inside same namespace as
// your Interface class.  It defines a shared pointer type.
#define WIRECELL_DEFINE_INTERFACE(ITYPE) \
    typedef std::shared_ptr<ITYPE> ITYPE##Ptr


namespace WireCell {

    class Interface {
    public:
	virtual ~Interface();
    };

    // an Interface is an Interface
    WIRECELL_DEFINE_INTERFACE(Interface);
}

#endif

