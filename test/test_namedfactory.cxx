#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Exceptions.h"

#include <iostream>
using namespace std;

class ISomeComponent : public virtual WireCell::IComponent<ISomeComponent> {
public:
    ISomeComponent() { cout << "SomeComponent() at " << (void*)this << endl; }
    virtual ~ISomeComponent() { cout << "~SomeComponent()" << endl; }
    virtual void chirp() = 0;
};

class SomeConcrete : public virtual ISomeComponent {
public:
    SomeConcrete() { cout << "SomeConcrete() at " << (void*)this << endl; }
    virtual ~SomeConcrete() { cout << "~SomeConcrete()" << endl; }
    virtual void chirp() { cout << "SomeConcrete::chirp() at " << (void*)this << endl; }
};

WIRECELL_FACTORY(SomeConcrete, SomeConcrete, ISomeComponent)


int main()
{
    // for this test we cheat since the factory isn't in a shared library.
    make_SomeConcrete_factory();

    auto ins = WireCell::Factory::lookup<ISomeComponent>("SomeConcrete");
    AssertMsg(ins, "Failed to lookup 'SomeConcrete' with interface 'ISomeComponent'");
    cerr << "Got SomeConcrete @ " << ins << endl;
    cerr << "Got bogus @ " << std::shared_ptr<ISomeComponent>() << endl;
    ins->chirp();

    bool caught = false;
    try {
        auto should_fail = WireCell::Factory::lookup<ISomeComponent>("NothingNamedThis");
    }
    catch (WireCell::FactoryException& e) {
        cerr << errstr(e) << endl;
        cerr << "^^^ Successfully failed to lookup a nonexistent component:\n";
             
        caught=true;
    }
    AssertMsg(caught, "Failed to throw");

    cerr << "Should not hear anything from this failed find_tn:\n";
    std::shared_ptr<ISomeComponent> ptr
        = WireCell::Factory::find_maybe_tn<ISomeComponent>("SomeConcrete:doesnotexist");
    AssertMsg(ptr==nullptr, "Got non null for nonexistent named component");

    cerr << "Success.\nNext, should the find should be successful:\n";
    std::shared_ptr<ISomeComponent> ptr2
        = WireCell::Factory::find_maybe_tn<ISomeComponent>("SomeConcrete");
    cerr << (void*)ptr2.get() << endl;
    AssertMsg(ptr2 != nullptr, "Got null for existing component");
}

