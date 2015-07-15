#ifndef WIRECELL_IFACTORY
#define WIRECELL_IFACTORY

#include "WireCellUtil/Interface.h"

namespace WireCell {

    class IFactory : public Interface {
    public:
	virtual ~IFactory();

	/// Create an instance of what we know how to create.
	virtual InterfacePtr create() = 0;

    };
    WIRECELL_DEFINE_INTERFACE(IFactory);

    class INamedFactory : public IFactory {
    public:
	
	virtual ~INamedFactory();

	/// Set name of class this factory can make
	virtual void set_classname(const std::string& name) = 0;
	/// Access name of class this factory can make
	virtual const std::string& classname() = 0;

	/// Create an instance by name.
	virtual InterfacePtr create(const std::string& name) = 0;

    };
    WIRECELL_DEFINE_INTERFACE(INamedFactory);

}

#endif

