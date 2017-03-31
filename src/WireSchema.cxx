#include "WireCellUtil/WireSchema.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Configuration.h"

using namespace WireCell;
using namespace WireCell::WireSchema;

Store WireCell::WireSchema::load(const char* filename)
{
    Json::Value jtop = WireCell::Persist::load(filename);
    Json::Value jstore = jtop["Store"];

    Store store;

    
    {// wires
        Json::Value jwires = jstore["wires"];
        const int nwires = jwires.size();
        std::vector<Wire>& wires = store.wires;
        wires.resize(nwires);
        for (int iwire=0; iwire<nwires; ++iwire) {
            Json::Value jwire = jwires[iwire];
            Wire& wire = wires[iwire];
            wire.ident = get<int>(jwire, "ident");
            wire.channel = get<int>(jwire, "channel");
            wire.segment = get<int>(jwire, "segment");

            Point& tail = wire.tail;
            Json::Value& jt = jwire["tail"];
            tail.set(convert<double>(jt[0]), convert<double>(jt[1]), convert<double>(jt[2]));

            Point& head = wire.head;
            Json::Value& jh = jwire["head"];
            head.set(convert<double>(jh[0]), convert<double>(jh[1]), convert<double>(jh[2]));
        }
    }

    
    {// planes
        Json::Value jplanes = jstore["planes"];
        const int nplanes = jplanes.size();
        std::vector<Plane>& planes = store.planes;
        planes.resize(nplanes);
        for (int iplane=0; iplane<nplanes; ++iplane) {
            Json::Value jplane = jplanes[iplane];
            Plane& plane = planes[iplane];
            plane.ident = get<int>(jplane, "ident");        
            Json::Value jwires = jplane["wires"];
            const int nwires = jwires.size();
            plane.wires.resize(nwires);
            for (int iwire=0; iwire<nwires; ++iwire) {
                plane.wires[iwire] = convert<int>(jwires[iwire]);
            }
        }
    }
    
    {// faces
        Json::Value jfaces = jstore["faces"];
        const int nfaces = jfaces.size();
        std::vector<Face>& faces = store.faces;
        faces.resize(nfaces);
        for (int iface=0; iface<nfaces; ++iface) {
            Json::Value jface = jfaces[iface];
            Face& face = faces[iface];
            face.ident = get<int>(jface, "ident");
            Json::Value jplanes = jface["planes"];
            const int nplanes = jplanes.size();
            face.planes.resize(nplanes);
            for (int iplane=0; iplane<nplanes; ++iplane) {
                face.planes[iplane] = convert<int>(jface[iplane]);
            }
        }
    }

    {// anodes
        Json::Value janodes = jstore["anodes"];
        const int nanodes = janodes.size();
        std::vector<Anode>& anodes = store.anodes;
        anodes.resize(nanodes);
        for (int ianode=0; ianode<nanodes; ++ianode) {
            Json::Value janode = janodes[ianode];
            Anode& anode = anodes[ianode];
            anode.ident = get<int>(janode, "ident");        
            Json::Value jfaces = janode["faces"];
            const int nfaces = jfaces.size();
            anode.faces.resize(nfaces);
            for (int iface=0; iface<nfaces; ++iface) {
                anode.faces[iface] = convert<int>(jfaces[iface]);
            }
        }
    }

    return store;
}


void WireCell::WireSchema::dump(const char* filename, const Store& store)
{

}


