#ifndef WIRECELL_NAMEDFACTORY_H
#define WIRECELL_NAMEDFACTORY_H

#include "WireCellUtil/IFactory.h"
#include "WireCellUtil/Singleton.h"
#include "WireCellUtil/PluginManager.h"
#include <unordered_map>

#include <iostream>//debug

namespace WireCell {
    /** A templated factory of objects of type Type that associates a
     * name to an object, returning a preexisting one if it exists. */
    template <class Type>
    class NamedFactory : public WireCell::INamedFactory {
    public:
	/// Remember the underlying type.
	typedef Type type;
	
	/// The exposed pointer type.
	typedef std::shared_ptr<Type> pointer_type;
	
	NamedFactory() : m_classname("") {}
	
	/// Return an instance associated with the given name.
	Interface::pointer create() { return create(""); }
	Interface::pointer create(const std::string& name) {
	    auto it = m_objects.find(name);
	    if (it == m_objects.end()) {
		pointer_type p(new Type);
		m_objects[name] = p;
		return p;
	    }
	    return it->second;
	}

	virtual void set_classname(const std::string& name) { m_classname=name; }
	virtual const std::string& classname() { return m_classname; }

    private:
	std::unordered_map<std::string, pointer_type> m_objects;
	std::string m_classname;
    };


    /** A registry of factories that produce instances which implement
     * a given interface. */
    template <class IType>
    class NamedFactoryRegistry {

    public:
	typedef IType interface_type;
	typedef std::shared_ptr<IType> interface_ptr;
	typedef std::shared_ptr<WireCell::INamedFactory> factory_ptr;

	/// Register an existing factory by the "class" name of the instance it can create.
	bool associate(const std::string& classname, factory_ptr factory) {
	    std::cerr << this << " associate " << classname << " with " << factory.get() << std::endl;

	    m_lookup[classname] = factory;
	    return true;
	}

	/// Look up an existing factory by the name of the "class" it can create.
	factory_ptr lookup_factory(const std::string& classname) {
	    auto it = m_lookup.find(classname);
	    if (it != m_lookup.end()) {
		return it->second;
	    }

	    // cache miss, try plugin
	    std::string factory_maker = "get_";
	    factory_maker += classname;
	    factory_maker += "_factory";
	    
	    auto pm = WireCell::Singleton< PluginManager >::Instance();
	    auto plug = pm.find(factory_maker.c_str());
	    if (plug) {
		typedef void (*maker_function)();
		maker_function mf;
		if (plug->symbol(factory_maker.c_str(), mf)) {
		    mf(); //  this should call associate() as a side effect!
		    auto hailmary = m_lookup.find(classname);
		    if (hailmary != m_lookup.end()) {
			return hailmary->second;
		    }
		}
	    }

	    std::cerr << "NFR: " << this << " Failed to lookup factory for '" << classname << "'" << std::endl;
	    return nullptr;
	}

	interface_ptr instance(const std::string& classname, const std::string& instname = "") {
	    factory_ptr fac = lookup_factory(classname);
	    if (!fac) { return nullptr; }
	    WireCell::Interface::pointer iptr = fac->create(instname);
	    if (!iptr) {
		std::cerr << "NFR: " << this << " Failed to find interface for '" << classname << "'::'" << instname << "'" << std::endl;
		return nullptr;
	    }
	    interface_ptr uptype = std::dynamic_pointer_cast<interface_type>(iptr);
	    //std::cerr << "Found interface for '" << classname << "'::'" << instname << "' @ " << uptype << std::endl;
	    return uptype;
	}

    public:
	typedef std::unordered_map<std::string, factory_ptr> factory_lookup;
	factory_lookup m_lookup;
    };    

    /// Singleton interface
    namespace Factory {

	template<class IType>
	bool associate(const std::string& classname, WireCell::INamedFactory::pointer factory) {
	    NamedFactoryRegistry<IType>&
		inst = WireCell::Singleton< NamedFactoryRegistry<IType> >::Instance();
	    std::cerr << "NFR: " << &inst << " associate(" << classname << ", " << factory << ")" << std::endl;
	    return inst.associate(classname, factory);
	}

	template<class IType>
	WireCell::INamedFactory::pointer lookup_factory(const std::string& classname) {
	    NamedFactoryRegistry<IType>&
		inst = WireCell::Singleton< NamedFactoryRegistry<IType> >::Instance();
	    std::cerr << "NFR: " << &inst << " lookup_factory(" << classname << ")" << std::endl;
	    return inst.lookup_factory(classname);
	}

	template<class IType>
	std::shared_ptr<IType> lookup(const std::string& classname, const std::string& instname="") {
	    NamedFactoryRegistry<IType>&
		inst = WireCell::Singleton< NamedFactoryRegistry<IType> >::Instance();
	    std::cerr << "NFR: " << &inst << " lookup(" << classname << ", " << instname << ")" << std::endl;
	    return inst.instance(classname, instname);
	}
    }

} // namespace WireCell




// #define WIRECELL_NAMEDFACTORY_CLASS(CLASS)				\
//     void associate_##CLASS##_factory(WireCell::NamedFactory< CLASS >::pointer factory)
    
#define WIRECELL_NAMEDFACTORY_INTERFACE(CLASS, INTERFACE)		\
    {									\
	if (!gs_##CLASS##_factory) {					\
	    gs_##CLASS##_factory = WireCell::NamedFactory< CLASS >::pointer(new WireCell::NamedFactory< CLASS >); \
	}								\
        std::cerr << "associate " << #CLASS << " with " << #INTERFACE << std::endl; \
	WireCell::Factory::associate<INTERFACE>(#CLASS, gs_##CLASS##_factory); \
    }



#define WIRECELL_NAMEDFACTORY(CLASS)					\
    static  WireCell::NamedFactory< CLASS >::pointer gs_##CLASS##_factory; \
    extern "C" void get_##CLASS##_factory()

// { \
//          factory(new  WireCell::NamedFactory< CLASS >);	\
// 	associate_##CLASS##_factory(factory);				\
//     }








// #define WIRECELL_NAMEDFACTORY(CLASS)					\
//     typedef WireCell::NamedFactory< CLASS > CLASS##Factory;		\
//     int force_link_##CLASS = -1;					\
//     extern "C" { int see_##CLASS##_force_link() { return force_link_##CLASS;}} \
//     CLASS##Factory::pointer register_##CLASS##_Factory() {		\
// 	CLASS##Factory::pointer p(new CLASS##Factory);			\
// 	p->set_classname(#CLASS);					\
// 	return p;							\
//     }									\
//     static CLASS##Factory::pointer gs_##CLASS##_factory = register_##CLASS##_Factory()

// #define WIRECELL_NAMEDFACTORY_ASSOCIATE(CLASS, INTERFACE)		\
//     static bool gs_##CLASS##_##INTERFACE##_assocation_ok =		\
// 	WireCell::Factory::associate<INTERFACE>(#CLASS, gs_##CLASS##_factory)

// #define WIRECELL_NAMEDFACTORY_USE(CLASS)	\
//     { extern int force_link_##CLASS;  force_link_##CLASS = 1;}
    

#endif
