#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Testing.h"

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

WIRECELL_NAMEDFACTORY(SomeConcrete);
WIRECELL_NAMEDFACTORY_ASSOCIATE(SomeConcrete, ISomeComponent);


int main()
{
    auto ins = WireCell::Factory::lookup<ISomeComponent>("SomeConcrete");
    AssertMsg(ins, "Failed to lookup 'SomeConcrete' with interface 'ISomeComponent'");
    cerr << "Got SomeConcrete @ " << ins << endl;
    cerr << "Got bogus @ " << std::shared_ptr<ISomeComponent>() << endl;
    ins->chirp();
}

