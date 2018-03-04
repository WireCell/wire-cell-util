/** A simple DFP engine meant for single-threaded execution.

    A node is constructed with zero or more ports.  

    A port mediates between a node and an edge.

    A port is either of type input or output.

    In the context of a node, a port has a name and an index into an
    ordered list of other ports of a given type.

    An edge is a queue passing data objects in one direction from its
    tail (input) end to its head (output) end.

    Each end of an edge is exclusively "plugged" into one node through
    one port. 
    
    A valid graph consists of nodes with all ports plugged to edges.

 */

#ifndef WIRECELL_UTIL_PIPEGRAPH
#define WIRECELL_UTIL_PIPEGRAPH

#include "WireCellUtil/Exceptions.h"


#include <boost/any.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace WireCell {
    namespace PipeGraph {

        // The type of data passed in the graph.
        typedef boost::any Data;
        // A buffer of data.
        typedef std::queue<Data> Queue;

        // Edges are just queues that can be shared.
        typedef std::shared_ptr<Queue> Edge;

        class Node;

        class Port {
        public:
            enum Type { tail=0, output=0, head=1, input=1, ntypes=2 };

            Port(Node* node, Type type, std::string name="") :
                m_node(node), m_type(type), m_name(name), m_edge(nullptr)
                { }
                
            bool isinput() { return m_type == Port::input; }
            bool isoutput() { return m_type == Port::output; }

            // Connect an edge, returning any previous one.
            Edge plug(Edge edge) {
                Edge ret = m_edge;
                m_edge = edge;
                return ret;
            }

            // return edge queue size or 0 if no edge has been plugged
            int size() {
                if (!m_edge) { return 0; }
                return m_edge->size();
            }

            // Return true if queue is empty or no edge has been plugged.
            bool empty() {
                if (!m_edge or m_edge->empty()) { return true; }
                return false;
            }

            Data get() {
                if (isoutput()) {
                    THROW(RuntimeError()
                          << errmsg{"can not get from output port"});
                }
                if (!m_edge) {
                    THROW(RuntimeError() << errmsg{"port has no edge"});
                }
                if (m_edge->empty()) {
                    THROW(RuntimeError() << errmsg{"edge is empty"});
                }
                Data ret = m_edge->front();
                m_edge->pop();
                return ret;
            }

            void put(Data& data) {
                if (isinput()) {
                    THROW(RuntimeError() << errmsg{"can not put to input port"});
                }
                if (!m_edge) {
                    THROW(RuntimeError() << errmsg{"port has no edge"});
                }
                m_edge->push(data);
            }

            const std::string& name() { return m_name; }

        private:
            std::string m_name;
            Type m_type;
            Edge m_edge;
            Node* m_node;       // node to which port belongs
        };

        typedef std::vector<Port> PortList;

        // A node in the DFP graph must inherit from Node.
        class Node {
        public:
            Node() {} // constructures may wish to resize/populate m_ports.
            virtual ~Node() { }
            
            // Concrete Node must implement this to consume inputs
            // and/or produce outputs.
            virtual bool operator()() = 0;

            // By default a Node is ready to execute if no input ports
            // are empty. Concrete ports may override.
            virtual bool ready()  {
                for (auto& p : m_ports[Port::input]) {
                    if (p.empty()) return false;
                }
                return true;
            }

            Port& iport(int ind=0) {
                return port(Port::input, ind);
            }
            Port& oport(int ind=0) {
                return port(Port::output, ind);
            }

            PortList& input_ports() {
                return m_ports[Port::input];
            }
            PortList& output_ports() {
                return m_ports[Port::output];
            }

            Port& port(Port::Type type, int ind=0) {
                if (ind < 0 or ind >= m_ports[type].size()) {
                    THROW(ValueError() << errmsg{"unknown port"});
                }
                return m_ports[type][ind];
            }
            Port& port(Port::Type type, const std::string& name) {
                for (int ind=0; ind<m_ports[type].size(); ++ind) {
                    if (m_ports[type][ind].name() != name) {
                        continue;
                    }
                    return port(type, ind);
                }
                THROW(ValueError() << errmsg{"unknown port"});
            }

        protected:
            // Concrete class should fill during construction
            PortList m_ports[Port::ntypes];
        };


        class Graph {
        public:
            // Connect two nodes by their given ports.
            void connect(Node* tail, Node* head, int tpind=0, int hpind=0) {
                m_edges.push_back(std::make_pair(tail,head));
                Edge edge = std::make_shared<Queue>();
                tail->port(Port::output, tpind).plug(edge);
                head->port(Port::input, hpind).plug(edge);                
            }

            // return a topological sort of the graph as per Kahn algorithm.
            std::vector<Node*> sort_kahn() {
                std::unordered_map< Node*, std::vector<Node*> > edges;
                std::unordered_map<Node*,int> nincoming;
                for (auto th : m_edges) {
                    edges[th.first].push_back(th.second);
                    nincoming[th.first] += 0; // make sure all nodes represented
                    nincoming[th.second] += 1;
                }
                
                std::vector<Node*> ret;
                std::unordered_set<Node*> seeds;

                for (auto it : nincoming) {
                    if (it.second == 0) {
                        seeds.insert(it.first);
                    }
                }

                while (!seeds.empty()) {
                    Node* t = *seeds.begin();
                    seeds.erase(t);
                    ret.push_back(t);

                    for (auto h : edges[t]) {
                        nincoming[h] -= 1;
                        if (nincoming[h] == 0) {
                            seeds.insert(h);
                        }
                    }
                }
                return ret;
            }

            bool execute() {
                auto nodes = sort_kahn();
                
                while (true) {
                    bool did_something = false;
                    // go through nodes starting outputs
                    for (auto nit = nodes.rbegin(); nit != nodes.rend(); ++nit) {
                        Node* node = *nit;
                        if (!node->ready()) {
                            continue; // go futher upstream
                        }
                        
                        bool ok = (*node)();
                        if (!ok) {
                            std::cerr << "PipeGraph failed\n";
                            return false;
                        }
                        did_something = true;
                        break;
                    }
                    if (!did_something) {
                        return true;
                    }
                }
                return true;    // shouldn't reach
            }

        private:
            std::vector<std::pair<Node*,Node*> > m_edges;
        };

        // class GraphSorter {
        // public:
        //     void add_edge(Node* tail, Node* head) {
        //         m_edges[tail].push_back(head);
        //         m_nincoming[tail] += 0;
        //         m_nincoming[head] += 1;
        //     }
            
        //     // topological sort by Kahn
        //     std::vector<Node*> operator()() {
        //         std::vector<Node*> ret;
        //         std::unordered_set<Node*> seeds;

        //         for (auto it : m_nincoming) {
        //             //std::cerr << "#incoming: " << it.second << std::endl;
        //             if (it.second == 0) {
        //                 seeds.insert(it.first);
        //             }
        //         }
        //         //std::cerr << "Sorting with " << seeds.size() << " seeds\n";

        //         while (!seeds.empty()) {
        //             Node* t = *seeds.begin();
        //             seeds.erase(t);
        //             ret.push_back(t);

        //             for (auto h : m_edges[t]) {
        //                 m_nincoming[h] -= 1;
        //                 if (m_nincoming[h] == 0) {
        //                     seeds.insert(h);
        //                 }
        //             }
        //         }
        //         return ret;
        //     }


        // private:
        //     // map tail to collection of heads
        //     std::unordered_map< Node*, std::vector<Node*> > m_edges;
        //     std::unordered_map<Node*,int> m_nincoming;

        // };



    }
}
#endif
