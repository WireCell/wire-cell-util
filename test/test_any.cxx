#include <boost/any.hpp>
#include <memory>
#include <iostream>

using namespace std;

struct Base { 
    virtual ~Base() {}
    int b; 
};
struct Foo : public Base {
    virtual ~Foo() {}
    int a;
};

int main()
{
    boost::any aobj;
    {
        aobj = std::make_shared<Foo>();
    }
    auto base = boost::any_cast<std::shared_ptr<Foo> >(aobj);

    if (!base) {
        cerr << "base is null\n";
    }
    else {
        cerr << "base is not null\n";
    }


    return 0;
    
}
