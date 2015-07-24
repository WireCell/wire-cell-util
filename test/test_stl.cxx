#include <vector>

struct Foo {
    int x;
};


int main()
{
    Foo* fp = 0;
    const Foo* cfp = fp;


    std::vector<int> xxx;
    std::vector<Foo*> foov;

    fp = new Foo;
    foov.push_back(fp);

    Foo* a = foov[0];

    Foo* b = *(foov.begin());

}
