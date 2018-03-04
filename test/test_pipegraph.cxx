#include "WireCellUtil/PipeGraph.h"

#include <iostream>

using namespace WireCell;
using namespace std;

class IdNode : public PipeGraph::Node
{
public:
    IdNode(const std::string& name, int id, int nin=0, int nout=0)
        : m_name(name), m_id(id) {
        using PipeGraph::Port;
        for (int ind=0; ind<nin; ++ind) {
            m_ports[Port::input].push_back(
                PipeGraph::Port(this, PipeGraph::Port::input));
        }
        for (int ind=0; ind<nout; ++ind) {
            m_ports[Port::output].push_back(
                PipeGraph::Port(this, PipeGraph::Port::output));
        }
    }        
    int id() { return m_id; }

    std::ostream& msg(const std::string s) {
        std::cerr << m_name << "[" << m_id << "]:\t" << s;
        return std::cerr;
    }
    

private:
    int m_id;
    std::string m_name;
};
class Source : public IdNode {
public:
    Source(int id, int beg, int end)
        : IdNode("src", id, 0, 1), m_num(beg), m_beg(beg), m_end(end) {}
    virtual ~Source() {}
    virtual bool ready()  {
        return m_num < m_end;
    }
    virtual bool operator()() {
        if (m_num >= m_end) {
            msg("dry\n");
            return true;
        }
        msg("make: ") << m_num << std::endl;
        PipeGraph::Data d = m_num;
        oport().put(d);
        ++m_num;
        return true;
    }
private:
    int m_num, m_beg, m_end;
};
class Sink: public IdNode {
public:
    Sink(int id) : IdNode("dst", id, 1, 0) {}
    virtual ~Sink() {}
    virtual bool operator()() {
        int d = boost::any_cast<int>(iport().get());
        msg("sink: ") << d << std::endl;
        return true;
    }
};
class Njoin: public IdNode {
public:
    Njoin(int id, int n) : IdNode("joi", id, n, 1) {}
    virtual bool ready()  {
        auto& ip = input_ports();
        for (int ind=0; ind<ip.size(); ++ind) {
            auto& p = ip[ind];
            if (p.empty()) {
                //msg("port empty: ") << ind << std::endl;
                return false;
            }
        }
        return true;
    }
    virtual bool operator()() {
        PipeGraph::Queue outv;
        auto& o = msg("join: ");
        for (auto p : input_ports()) {
            PipeGraph::Data d = p.get();
            int n = boost::any_cast<int>(d);
            o << n << " ";
            outv.push(d);
        }
        o << std::endl;

        PipeGraph::Data out = outv;
        oport().put(out);
        return true;
    }
};
class SplitQueueBuffer : public IdNode {
public:
    SplitQueueBuffer(int id) : IdNode("sqb", id, 1, 1) {}
    virtual bool read() {
        if (!m_buf.empty()) {
            return true;
        }
        return IdNode::ready();
    }
    virtual bool operator()() {
        if (m_buf.empty()) {
            m_buf = boost::any_cast<PipeGraph::Queue>(iport().get());
        }
        auto d = m_buf.front();
        m_buf.pop();
        oport().put(d);
        return true;
    }
private:
    PipeGraph::Queue m_buf;
};

class Nfan : public IdNode {
public:
    Nfan(int id, int n) : IdNode("fan", id, 1, n) {}
    virtual bool operator()() {
        auto obj = iport().get();
        int d = boost::any_cast<int>(obj);
        msg("nfan: ") << d << std::endl;
        for (auto p : output_ports()) {
            p.put(obj);
        }
        return true;
    }
};
class Func : public IdNode
{
public:
    Func(int id) : IdNode("fun", id, 1, 1) {}
    virtual bool operator()() {
        PipeGraph::Data out = iport().get();
        int d = boost::any_cast<int>(out);
        msg("func: ") << d << std::endl;
        oport().put(out);
        return true;
    }
};

// also: N->M: hydra


int main() {
    using PipeGraph::Node;
    using PipeGraph::Graph;

    int count = 0;
    auto src1 = new Source(count++, 0,4);
    auto src2 = new Source(count++, 10,14);
    auto dst1 = new Sink(count++);
    auto dst2 = new Sink(count++);
    auto fun1 = new Func(count++);
    auto fun2 = new Func(count++);
    auto fun3 = new Func(count++);
    auto fan1 = new Nfan(count++, 2);
    auto joi1 = new Njoin(count++, 2);
    auto sqb1 = new SplitQueueBuffer(count++);
    

    Graph g;
    g.connect(src1,fun1);
    g.connect(fun1,fan1);
    g.connect(fan1,dst1);
    g.connect(fan1,fun2,1);
    g.connect(fun2,joi1);
    g.connect(src2,fun3);
    g.connect(fun3,joi1,0,1);
    g.connect(joi1,sqb1);
    g.connect(sqb1,dst2);

    auto sorted = g.sort_kahn();
    cout << "Sorted to " << sorted.size() << " nodes\n";

    for (int ind=0; ind<sorted.size(); ++ind) {
        IdNode* idn = dynamic_cast<IdNode*>(sorted[ind]);
        idn->msg("at index ") << ind << endl;
    }
    cout << "Executing:\n";

    g.execute();

}
