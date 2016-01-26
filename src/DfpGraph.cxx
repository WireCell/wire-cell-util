#include "WireCellUtil/DfpGraph.h"


using namespace WireCell;
    
DfpGraph::Vertex DfpGraph::get_add_vertex(const VertexProperty& tn)
{
    auto it = vertex_property_map.find(tn);
    if (it != vertex_property_map.end()) {
	return it->second;
    }
    auto v = add_vertex(tn, graph);
    vertex_property_map[tn] = v;
    return v;
}
	
DfpGraph::Edge DfpGraph::connect(const std::string& tail, const std::string& head,
				 int tailport, int headport)
{
    VertexProperty tvp(tail);
    VertexProperty hvp(head);

    auto tv = get_add_vertex(tvp);
    auto hv = get_add_vertex(hvp);

    EdgeProperty ep(tailport, headport);
    Edge e; bool b;
    std::tie(e, b) = boost::add_edge(tv, hv, ep, graph);
    return e;
}

std::vector<DfpGraph::Connection> DfpGraph::connections()
{

    std::vector<Connection> ret;
    auto vits = boost::vertices(graph);
    for (auto v = vits.first; v != vits.second; ++v) {
	auto vp = graph[*v];
	auto eits = out_edges(*v, graph);

	if (eits.first == eits.second) continue;

	for (auto e = eits.first; e != eits.second; ++e) {
	    auto ep = graph[*e];

	    auto other = target(*e, graph);
	    auto hp = graph[other];

	    ret.push_back(std::make_tuple(vp,hp,ep));
	}
    }
    return ret;
}

