#include "WireCellUtil/RayCluster.h"
#include "WireCellUtil/Waveform.h"

#include <iostream>

using namespace WireCell;
using namespace WireCell::Waveform;
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
        const auto corners = c.corners();
        cerr << "\t" << corners.size() << " corners:";
        for (const auto& cor : corners) {
            cerr << " (["
                 << cor.first.rccs
                 << ","
                 << cor.first.grid
                 << "],["
                 << cor.second.rccs
                 << ","
                 << cor.second.grid
                 << "])";
        }
        cerr << endl;
    }
}

int main()
{
    const double width=100, height=100, pitch_mag = 5;
    auto raypairs = make_raypairs(width, height, pitch_mag);
    const int nlayers = raypairs.size();

    RayGrid rg(raypairs);
    const auto& pitches = rg.pitch_dirs();
    const auto& centers = rg.centers();
    const auto& pitch_mags = rg.pitch_mags();

    RayClustering rc(rg);

    std::vector< std::vector<float> > measures(nlayers);

    const std::vector<Point> points{ Point(0, 10, 10), Point(0, 10, 11), Point(0, 20, 20) };

    for (const auto& p : points) {
        for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
            // pimpos normally would help here to find pitch location of arb point
            const auto& pit = pitches[ilayer];
            const auto& cen = centers[ilayer];
            const auto rel = p-cen;
            const int pit_ind = pit.dot(rel)/pitch_mags[ilayer]; 
            auto& m = measures[ilayer];
            if ((int)m.size() <= pit_ind) {
                m.resize(pit_ind+1, 0.0);
            }

            m[pit_ind] += 1.0;

            // {
            //     cerr << ilayer
            //          << " pitind=" << pit_ind
            //          << " p="<<p << " rel=" << rel
            //          << " pit="<< pit << " cen=" << cen
            //          << " tot=" << std::accumulate(m.begin(), m.end(), 0.0)
            //          << " m.size=" << m.size()
            //          << endl;
            // }
        }
    }
        
    RayClustering::clustering_t clusters;

    for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
        auto& m = measures[ilayer];

        for (size_t ind=0; ind<m.size(); ++ind) {
            if (m[ind] <= 0.0) { continue; }
            std::cerr << ilayer << "[" << ind << "]=" << m[ind]<<endl;
        }

        RayClustering::Activity activity{ilayer, m.begin(), std::make_pair(m.begin(), m.end())};

        auto strips = activity.make_strips();

        auto tot = std::accumulate(m.begin(), m.end(), 0.0);

        cerr << "Layer: " << activity.layer
             << " activity=" << tot
             << " in: " << strips.size() << " strips"  << endl;

        if (clusters.empty()) {
            clusters = rc.cluster(activity);
        }
        else {
            clusters = rc.cluster(clusters, activity);
        }
        dump(clusters);
    }


    return 0;
}
