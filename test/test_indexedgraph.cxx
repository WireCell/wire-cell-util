#include "WireCellUtil/IndexedGraph.h"
#include "WireCellUtil/Testing.h"

#include <boost/graph/graphviz.hpp>

using namespace WireCell;

char foo(size_t ind)
{
    return "if"[ind];
}

int main()
{
    typedef std::shared_ptr<int> iptr_t;
    typedef std::shared_ptr<float> fptr_t;
    typedef std::variant<iptr_t, fptr_t> if_t;

    std::unordered_map<if_t, int> p2i;

    typedef IndexedGraph<if_t> indexed_graph_t;
    indexed_graph_t g;
    if_t one = std::make_shared<int>(42);
    if_t two = std::make_shared<float>(6.9);
    if_t tre = std::make_shared<float>(33);

    p2i[one] = 1;
    p2i[two] = 2;
    p2i[tre] = 3;

    // test variant
    Assert(1 == tre.index());
    Assert(*std::get<1>(two) < *std::get<1>(tre));
    Assert(one < two);      // caution: compares against indices first!
    Assert(*std::get<0>(one) == 42);
    Assert(nullptr == std::get_if<1>(&one));
    Assert('i' == foo(0));
    Assert('f' == foo(1));

    auto v1 = g.vertex(one);
    auto v2 = g.vertex(one);
    assert(v1 == v2);
    g.edge(one, two);
    g.edge(one, two);
    g.edge(two, one);
    g.edge(one, tre);
    auto verts = boost::vertices(g.graph());
    Assert(verts.second-verts.first == 3);


    indexed_graph_t g2(g.graph());

    std::vector<std::string> names{"one","two","tre"};
    std::unordered_map<indexed_graph_t::vdesc_t, std::string> ids;
    for (auto u : boost::make_iterator_range(vertices(g2.graph()))) {
        ids[u] = names[ids.size()];
    }
    boost::default_writer w;
    boost::write_graphviz(std::cout,
                          g2.graph(),
                          w,w,w,
                          boost::make_assoc_property_map(ids));

    return 0;

}
