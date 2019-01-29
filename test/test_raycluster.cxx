#include "WireCellUtil/RayCluster.h"

#include <iostream>

using namespace WireCell;
using namespace std;

#include "raygrid.h"

void dump(const RayClustering::clustering_t& clusters)
{
    cerr << clusters.size() << " clusters:\n";
    for (const auto& c : clusters) {
        const auto& strips = c.strips();
        cerr << "\t" << strips.size() << " strips:";
        for (const auto& s : strips) {
            cerr << " " << s.layer << " [" << s.bounds.first << "," << s.bounds.second << "]";
        }
        cerr << endl;
    }
}

int main()
{
    auto raypairs = make_raypairs();
    RayGrid rg(raypairs);
    RayClustering rc(rg);

    std::vector<float> measure(100,0.0);

    measure[3] = 1.0;
    measure[30] = 1.0;
    measure[31] = 1.0;
    measure[32] = 1.0;

    auto mbeg = measure.begin();
    RayClustering::Activity activity{0, mbeg, std::make_pair(mbeg+2, mbeg+35)};
    auto clusters = rc.cluster(activity);
    dump(clusters);

    return 0;
}
