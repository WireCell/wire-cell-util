/**
   Code here helps read in data which follows the Wire Cell Toolkit
   wire data schema.  See python module wirecell.util.wires.schema and
   sister submodules.

 */
#ifndef WIRECELLUTIL_WIREPLANES
#define WIRECELLUTIL_WIREPLANES

#include "WireCellUtil/Point.h"


namespace WireCell {

    namespace WireSchema {

        struct Wire {
            int ident;
            int channel;
            int segment;
            Point tail, head;   // end points, direction of signal to channel
        };
            
        struct Plane {
            int ident;
            std::vector<int> wires;
        };

        struct Face {
            int ident;
            std::vector<int> planes;
        };

        struct Anode {
            int ident;
            std::vector<int> faces;
        };

        struct Store {
            std::vector<Anode> anodes;
            std::vector<Face> faces;
            std::vector<Plane> planes;
            std::vector<Wire> wires;
        };



        Store load(const char* filename);
        void dump(const char* filename, const Store& store);

    }

}
#endif
