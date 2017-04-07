#include "WireCellUtil/WireSchema.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Configuration.h"

#include <iostream>             // debug

using namespace WireCell;
using namespace WireCell::WireSchema;

Store WireCell::WireSchema::load(const char* filename)
{
    Json::Value jtop = WireCell::Persist::load(filename);
    Json::Value jstore = jtop["Store"];

    Store store;

    
    
    std::vector<Point> points;
    {
        Json::Value jpoints = jstore["points"];
        const int npoints = jpoints.size();
        points.resize(npoints);
        //std::cerr << "Points: " << npoints << std::endl;
        for (int ipoint=0; ipoint<npoints; ++ipoint) {
            Json::Value jp = jpoints[ipoint]["Point"];
            points[ipoint].set(get<double>(jp,"x"), get<double>(jp,"y"), get<double>(jp,"z"));
        }        
    }

    {// wires
        Json::Value jwires = jstore["wires"];
        const int nwires = jwires.size();
        std::vector<Wire>& wires = store.wires;
        wires.resize(nwires);
        //std::cerr << "Wires: " << nwires << std::endl;
        for (int iwire=0; iwire<nwires; ++iwire) {
            Json::Value jwire = jwires[iwire]["Wire"];
            Wire& wire = wires[iwire];
            wire.ident = get<int>(jwire, "ident");
            wire.channel = get<int>(jwire, "channel");
            wire.segment = get<int>(jwire, "segment");

            int itail = get<int>(jwire,"tail");
            int ihead = get<int>(jwire,"head");
            // std::cerr << "wire:" << iwire  << " tail:" << itail << " head:"<<ihead
            //           << " npoints:" << points.size()
            //           << std::endl;
            wire.tail = points[itail];
            wire.head = points[ihead];
        }
    }

    
    {// planes
        Json::Value jplanes = jstore["planes"];
        const int nplanes = jplanes.size();
        std::vector<Plane>& planes = store.planes;
        planes.resize(nplanes);
        //std::cerr << "Planes: " << nplanes << std::endl;
        for (int iplane=0; iplane<nplanes; ++iplane) {
            Json::Value jplane = jplanes[iplane]["Plane"];
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
        //std::cerr << "Faces: " << nfaces << std::endl;
        for (int iface=0; iface<nfaces; ++iface) {
            Json::Value jface = jfaces[iface]["Face"];
            Face& face = faces[iface];
            face.ident = get<int>(jface, "ident");
            Json::Value jplanes = jface["planes"];
            const int nplanes = jplanes.size();
            face.planes.resize(nplanes);
            for (int iplane=0; iplane<nplanes; ++iplane) {
                face.planes[iplane] = convert<int>(jplanes[iplane]);
            }
        }
    }

    {// anodes
        Json::Value janodes = jstore["anodes"];
        const int nanodes = janodes.size();
        std::vector<Anode>& anodes = store.anodes;
        anodes.resize(nanodes);
        //std::cerr << "Anodes: " << nanodes << std::endl;
        for (int ianode=0; ianode<nanodes; ++ianode) {
            Json::Value janode = janodes[ianode]["Anode"];
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


