#include "WireCellUtil/DfpGraph.h"
#include "WireCellUtil/Configuration.h"

#include <iostream>

using namespace std;

int main()
{
    using namespace WireCell;
    DfpGraph dfp;

    std::string json = R"(
[
[["A:a"],["B"]],
[["A:a"],["B:b"]],
[["A:a",1],["B:b",1]],
[["A:a",2],["C:c",1]],
[["A:a",2],["D:d",1]],
[["C:c",1],["D:d",2]]
]
)";
    auto cfg = configuration_loads(json, "json");

    for (auto conn : cfg) {
	auto tail = conn[0];
	auto head = conn[1];
	dfp.connect(convert<string>(tail[0]), convert<string>(head[0]),
		    convert<int>(tail[1]), convert<int>(head[1]));
    }

    for (auto thc: dfp.connections()) {
	auto tail = get<0>(thc);
	auto head = get<1>(thc);
	auto conn = get<2>(thc);
	cerr << tail << " " << conn << " " << head << endl;
    }

    // auto vits = boost::vertices(dfp.graph);
    // for (auto v = vits.first; v != vits.second; ++v) {
    // 	auto vp = dfp.graph[*v];
    // 	auto eits = out_edges(*v, dfp.graph);

    // 	if (eits.first == eits.second) continue;

    // 	cerr << "Got: " << vp.type << ":" << vp.name << " @ " << *v << endl;

    // 	for (auto e = eits.first; e != eits.second; ++e) {
    // 	    auto ep = dfp.graph[*e];

    // 	    auto other = target(*e, dfp.graph);
    // 	    auto hp = dfp.graph[other];

    // 	    cerr << "Conn: "
    // 		 << vp.type << ":" << vp.name << "[" << ep.tail << "] --> "
    // 		 << hp.type << ":" << hp.name << "[" << ep.head << "]\n";
    // 	}
    // }



    // auto v1 = boost::add_vertex(TypeName("type1","name1"), g);
    // auto v2 = boost::add_vertex(TypeName("type2","name2"), g);
    // boost::add_edge(v1, v2, EdgeProperty(1,2), g);
    // auto e12 = *out_edges(v1, g).first;

    // cerr << WireCell::type(e12) << endl
    // 	 << g[e12].tail << " --> " << g[e12].head << endl
    // 	 << g[v1].type << ":" << g[v1].name << endl;
    return 0;
}
