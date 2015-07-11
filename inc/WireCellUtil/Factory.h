/**
 * The factory pattern implemented here allows for an instance of a
 * concrete class to be created by specifying a factory name which is
 * associated with the concrete class type and an instance name which
 * is used to associated with one instance.
 *
 * This decouples code that wants to use a instance from directly
 * compiling against the code that implements the instance.  It also
 * allows for a centralized configuration mechanism.
 *
 * This factory pattern is used for producing concrete instances of
 * many of the "larger" interface classes in WireCellIface.  As such,
 * this particular implementation assumes a two tiered hierarchy where
 * concrete instances are accessed through particular base classes
 * (eg, the Interface).  This means there is a single factory registry
 * instance for each base class which matters.  For each con concrete
 * subclass one must arrange to register a subclass factory.
 * 
 * A "simple" factory is one which creates instances with their
 * default constructor.  If this is adequate for your class you can
 * place the following in the implementation file of your class at top
 * level:
 *
 *    #include "WireCellUtil/Factory.h"
 *    static auto gsfactory = WireCell::factory_simple_registry<MyClass, Interface>("MyFactoryName");
 *
 * Where `MyClass` is the concrete subclass of the base `Interface`
 * class.
 *
 * If your class requires a non-default constructor you will have to
 * write your own explicit class.  It is recommended to take a look at
 * WireCell::SimpleFactory below as an example and implement
 * `create()` to do what ever is needed.
 *
 * If you need to pass in configuration information then do not invent
 * some way to do this yourself but instead also have your class
 * inherit from WireCell::IConfigurable
 */

#ifndef WIRECELLUTIL_FACTORY
#define WIRECELLUTIL_FACTORY

#include "WireCellUtil/Singleton.h"

#include <string>
#include <unordered_map>

namespace WireCell {

    
    /** A base class for a caching factory returning an instance of
     * type T by name.
     *
     * Actual creation is delegated to subclasses.
     */
    template<class B>
    class BaseFactory {
	std::unordered_map<std::string, B*> instances;
    public:
	virtual ~BaseFactory() {};

	/// Return the instance of the given name, creating if necessary.
	B* get(const std::string& name = "") {
	    B* inst = instances[name];
	    if (inst) { return inst; }
	    inst = this->create();
	    instances[name] = inst;
	    return inst;
	}

	/// Subclass must implement.
	virtual B* create() = 0;
    };


    /** A simple concrete factory that creates a default instance of
     * templated type T. */
    template<class T, class B>
    class SimpleFactory : public BaseFactory<B> {
    public:
	virtual ~SimpleFactory() {};

	B* create() { return new T; }
    };


    /** A registry of factories producing instances of base class T.*/
    template<class T>
    class FactoryRegistry {
    public:
	typedef BaseFactory<T> Factory;

	virtual ~FactoryRegistry() {};

	void add(Factory* factory, const std::string& factory_name) {
	    factories[factory_name] = factory;
	}
	Factory* get(const std::string& factory_name) {
	    return factories[factory_name];
	}

    private:
	std::unordered_map<std::string, Factory*> factories;
    };

    /// just rewording of accessing the singleton
    // template<class B>
    // typename FactoryRegistry<B>& factory_registry() {
    // 	return Singleton< FactoryRegistry<B> >::Instance();
    // }


    /** Return a factory registry for tempated type and of the given
     * name or 0. */
    template<class B>
    typename FactoryRegistry<B>::Factory* factory_get_factory(const std::string& factory_name) {
	typedef FactoryRegistry<B> FR;
	FR& fr = Singleton<FR>::Instance();
	return fr.get(factory_name);
    }

    /** Return a named instance of templated type B from named factory. */
    template<class B>
    B* factory_instance(const std::string& factory_name, const std::string& instance_name = "") {
	typename FactoryRegistry<B>::Factory* fact = factory_get_factory<B>(factory_name);
	if (!fact) { return 0; }
	return fact->get(instance_name);	
    }

    /** Helper function to perform a registration of a simple factory
     * that creates instances with their default constructor. */
    template<class T, class B>
    typename FactoryRegistry<B>::Factory* factory_simple_registry(const std::string& factory_name) {
	typedef FactoryRegistry<B> FR;
	FR& fr = Singleton< FR >::Instance();
	typename FR::Factory* fact = new SimpleFactory<T,B>;
	fr.add(fact, factory_name);
	return fact;
    }

}
#endif
