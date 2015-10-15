/** try to figure out how to mix OOP and GP.
 *
 * We have two objects known by base class Port that must be passed to
 * a templated function transfer() as downcast to types that carry the
 * argument type.
 *
 * To allow this to work the call to transfer() must be done in a
 * context that can do this down cast.  This context must located
 * dynamically based on type_info.  So, we preregister a bunch of
 * these contexts in a std::map by their type_info name.  
 */

#include "WireCellUtil/Testing.h"
#include <iostream>
#include <typeinfo>
#include <map>
#include <string>

using namespace std;

struct Port {
    virtual ~Port() {};
    virtual std::string port_type_name() const = 0;
}; 

template<typename T>
struct PortT : public Port {
    typedef T port_type;
    virtual ~PortT() {}
    virtual std::string port_type_name() const {
	return typeid(port_type).name();
    }
    virtual bool put(const port_type& in) { return false; }
    virtual bool get(port_type& out) const { return false; }
    virtual T make() const { T t; return t; }
};

struct SubI : public PortT<int> {
    virtual ~SubI() {}
    virtual bool get(port_type& out) const {
	out = 42;
	cerr << "SubI::get(" << out << ")" << endl;
	return true;
    }
};
struct SubF : public PortT<float> {
    virtual ~SubF() {}
    virtual bool get(port_type& out) const {
	out = 6.9;
	cerr << "SubF::get(" << out << ")" << endl;
	return true;
    }
};

struct SubFIn : public PortT<float> {
    virtual ~SubFIn() {}
    virtual bool put(const port_type& in) {
	cerr << "SubFIn::put(" << in << ")" << endl;
	return true;
    }
    virtual bool get(port_type& out) const {
	cerr << "SubIn::get() can not provide data" << endl;
	return true;
    }

};

struct SubFOut : public PortT<float> {
    virtual ~SubFOut() {}
    virtual bool put(const port_type& in) {
	cerr << "SubFOut::put(" << in << ") can not accept data" << endl;
	return false;
    }
    virtual bool get(port_type& out) const {
	out = 6.9;
	cerr << "SubFOut::get(" << out << ")" << endl;
	return true;
    }
};


template<typename A, typename B>
bool transfer(const A& a, B& b)
{
    cerr << "transfer: "
	 << typeid(A).name() << "<->" << typeid(a).name() << " "
	 << typeid(B).name() << "<->" << typeid(b).name() << endl;


    if (a.port_type_name() != b.port_type_name()) {
    	cerr << "Port type mismatch: "
    	     << a.port_type_name() << " != " << b.port_type_name() << endl;
    	return false;
    }
    typename A::port_type dat;
    if (!a.get(dat)) {
    	cerr << "Failed to get output of type " << a.port_type_name() << endl;
    	return false;
    }
    if (!b.put(dat)) { 
    	cerr << "Failed to put input of type " << b.port_type_name() << endl;
    	return false; 
    }
    return true;
}

struct CallTransfer {
    virtual ~CallTransfer() {}
    virtual bool call(const Port& a, Port& b) = 0;
};

template<typename T>
struct CallTransferT : public CallTransfer{
    typedef T port_type;
    virtual ~CallTransferT() {}
    std::string port_type_name() const {
	return typeid(port_type).name();
    }
    bool call(const Port& a, Port& b) {
	const PortT<port_type>* pa = dynamic_cast<const PortT<port_type>*>(&a);
	PortT<port_type>* pb = dynamic_cast<PortT<port_type>*>(&b);
	return transfer(*pa, *pb);
    }
};


static map<string, CallTransfer*> callers;

template<typename T>
void register_caller()
{
    CallTransferT<T> *ct = new CallTransferT<T>;
    callers[ct->port_type_name()] = ct;
}

CallTransfer* get_caller(Port& port)
{
    return callers[port.port_type_name()];
}


int main()
{

    Port* si = new SubI;
    Port* sf = new SubF;
    Port* sfi = new SubFIn;
    Port* sfo = new SubFOut;

    cout << "typeid(si) = " << typeid(si).name() << endl;
    cout << "typeid(*si) = " << typeid(*si).name() << endl;
    cout << "typeid(sf) = " << typeid(sf).name() << endl;
    cout << "typeid(*sf) = " << typeid(*sf).name() << endl;
    cout << "typeid(sfo) = " << typeid(sfo).name() << endl;
    cout << "typeid(*sfo) = " << typeid(*sfo).name() << endl;
    cout << "typeid(sfi) = " << typeid(sfi).name() << endl;
    cout << "typeid(*sfi) = " << typeid(*sfi).name() << endl;

    cout << "typeid(SubI::port_type) = " << typeid(SubI::port_type).name() << endl;
    cout << "typeid(SubF::port_type) = " << typeid(SubF::port_type).name() << endl;
    cout << "typeid(SubFOut::port_type) = " << typeid(SubFOut::port_type).name() << endl;
    cout << "typeid(SubFIn::port_type) = " << typeid(SubFIn::port_type).name() << endl;
    
    cout << "si->type_name() = " << si->port_type_name() << endl;
    cout << "sf->type_name() = " << sf->port_type_name() << endl;
    cout << "sfo->type_name() = " << sfo->port_type_name() << endl;
    cout << "sfi->type_name() = " << sfi->port_type_name() << endl;

    // given just Port*, how to know what to dynamic_cast to???
    PortT<float>* bfi = dynamic_cast<PortT<float>*>(sfi);
    PortT<float>* bfo = dynamic_cast<PortT<float>*>(sfo);
    Assert(bfi);
    Assert(bfo);
    Assert(transfer(*bfo, *bfi));

    // okay, like this:
    register_caller<int>();
    register_caller<float>();
    CallTransfer* ct = get_caller(*sfi);
    Assert(ct);
    Assert(ct->call(*sfo, *sfi));

    return 0;
}
