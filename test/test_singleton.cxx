#include "WireCellUtil/Singleton.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
using namespace std;

class Foo {
public:
    Foo() { cout << "Foo() at " << (void*)this << endl; }
    virtual ~Foo() { cout << "~Foo() at " << (void*)this << endl; }
    virtual void chirp() { cout << "Foo::chirp() at " << (void*)this << endl; }
};

typedef WireCell::Singleton<Foo> OnlyFoo;

int main()
{
    cout << "First time:" << endl;
    Foo* foo1 = &OnlyFoo::Instance();
    Foo* foo2 = &OnlyFoo::Instance();

    WireCell::Assert(foo1 == foo2);

    OnlyFoo::Instance().chirp();
    cout << "Second time:" << endl;
    OnlyFoo::Instance().chirp();
}
