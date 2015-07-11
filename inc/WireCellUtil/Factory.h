#ifndef WIRECELLUTIL_FACTORY
#define WIRECELLUTIL_FACTORY

#include "WireCellUtil/Singleton.h"

#include <string>
#include <unordered_map>

namespace WireCell {

    
    /** A base class for a caching factory returning an instance of type T by name.
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
