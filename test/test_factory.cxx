#include "WireCellUtil/Factory.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
using namespace std;

class SomeBase {
public:
    SomeBase() { cout << "SomeBase() at " << (void*)this << endl; }
    virtual ~SomeBase() { cout << "~SomeBase()" << endl; }
    virtual void chirp() { cout << "SomeBase::chirp() at " << (void*)this << endl; }
};
class SomeConcrete : public SomeBase {
public:
    SomeConcrete() { cout << "SomeConcrete() at " << (void*)this << endl; }
    virtual ~SomeConcrete() { cout << "~SomeConcrete()" << endl; }
    virtual void chirp() { cout << "SomeConcrete::chirp() at " << (void*)this << endl; }
};

// Must register a factory for the concrete subclass.  If it is a
// simple factory (just doing "new") then there is a helper function.
// We do it at static file scope to emulate who this would typically
// be done in a library.  What this helper does gets duplicated in
// "long hand" at the top of main() in order to give an example of how
// one might substitute a non-simple factory.
static auto gsfactory = WireCell::factory_simple_registry<SomeConcrete, SomeBase>("Cement");


int main()
{
    typedef WireCell::FactoryRegistry<SomeBase> SomeFactoryRegistry;
    SomeFactoryRegistry& fr = WireCell::Singleton< SomeFactoryRegistry >::Instance();
    auto concrete_factory = new WireCell::SimpleFactory<SomeConcrete, SomeBase>;
    fr.add(concrete_factory, "Cement");

    auto factory = fr.get("Cement");
    WireCell::Assert(concrete_factory == factory);
    
    SomeBase* def = factory->get();
    SomeBase* foo = factory->get("foo");
    WireCell::Assert(def != foo, "default is not different from 'foo'");
    def->chirp();
    foo->chirp();

    SomeBase* foo2 = factory->get("foo");
    WireCell::Assert(foo == foo2, "same name different pointer");
    foo2->chirp();

    SomeBase* foo3 = WireCell::factory_instance<SomeBase>("Cement","foo");
    WireCell::Assert(foo == foo3, "helper function sucks");

    SomeBase* bar = WireCell::factory_instance<SomeBase>("Cement","bar");
    WireCell::Assert(foo != bar, "different names, same instance");

    return 0;
}
