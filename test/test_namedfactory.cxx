#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
using namespace std;

class ISomeInterface : public virtual WireCell::TypedInterface<ISomeInterface> {
public:
    ISomeInterface() { cout << "SomeInterface() at " << (void*)this << endl; }
    virtual ~ISomeInterface() { cout << "~SomeInterface()" << endl; }
    virtual void chirp() = 0;
};

class SomeConcrete : public virtual ISomeInterface {
public:
    SomeConcrete() { cout << "SomeConcrete() at " << (void*)this << endl; }
    virtual ~SomeConcrete() { cout << "~SomeConcrete()" << endl; }
    virtual void chirp() { cout << "SomeConcrete::chirp() at " << (void*)this << endl; }
};

WIRECELL_NAMEDFACTORY(SomeConcrete);
WIRECELL_NAMEDFACTORY_ASSOCIATE(SomeConcrete, ISomeInterface);


int main()
{
    auto ins = WireCell::Factory::lookup<ISomeInterface>("SomeConcrete");
    AssertMsg(ins, "Failed to lookup 'SomeConcrete' with interface 'ISomeInterface'");
    cerr << "Got SomeConcrete @ " << ins << endl;
    cerr << "Got bogus @ " << std::shared_ptr<ISomeInterface>() << endl;
    ins->chirp();
}

