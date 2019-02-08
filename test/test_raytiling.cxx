#include "WireCellUtil/RayTiling.h"
#include "WireCellUtil/Waveform.h"

#include "TCanvas.h"
#include "TMarker.h"
#include "TText.h"
#include "TLatex.h"
#include "TLine.h"
#include "TPolyLine.h"
#include "TArrow.h"
#include "TH1F.h"

#include <math.h> 

#include <random>
#include <iostream>

using namespace WireCell;
using namespace WireCell::Waveform;
using namespace WireCell::RayGrid;
using namespace std;

// local helper codes
#include "raygrid.h"
#include "raygrid_draw.h"

int main(int argc, char* argv[])
{
    Printer print(argv[0]);

    auto raypairs = make_raypairs(width, height, pitch_magnitude);
    const int nlayers = raypairs.size();

    Coordinates coords(raypairs);

    draw_raygrid(print, coords, raypairs);

    const auto& pitches = coords.pitch_dirs();
    const auto& centers = coords.centers();
    const auto& pitch_mags = coords.pitch_mags();

    Tiling tiling(coords);

    std::vector< std::vector<Activity::value_t> > measures(nlayers);

    std::default_random_engine generator;
    std::uniform_real_distribution<double> position(0,std::max(width,height));
    std::normal_distribution<double> spread(0.0, gaussian);
    std::vector<Point> points;
    for (int idepo=0;idepo<ndepos;++idepo) {
        Point cp(0, position(generator), position(generator));
        for (int iele=0; iele<neles; ++iele) {
            Point delta(0, spread(generator), spread(generator));
            points.push_back(cp+delta);
        }
    }

    draw_frame(print.canvas, "Points and Activity");
    for (size_t ipt=0; ipt<points.size(); ++ipt ) {
        const auto& p = points[ipt];
        draw_point(p, 1, 24, ipt+1);
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
    for (int ilayer=0; ilayer<nlayers; ++ilayer) {
        draw_layer(coords, ilayer, pitch_mags[ilayer],
                   pitches[ilayer], centers[ilayer], measures[ilayer]);
    }
    print();
        
    blobs_t blobs;

    draw_frame(print.canvas, "Points and Strips");
    for (size_t ipt=0; ipt<points.size(); ++ipt ) {
        const auto& p = points[ipt];
        draw_point(p, 1, 24, ipt+1);
    }
    activities_t activities;
    for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
        auto& m = measures[ilayer];

        for (size_t ind=0; ind<m.size(); ++ind) {
            if (m[ind] <= 0.0) { continue; }
            std::cerr << ilayer << "[" << ind << "]=" << m[ind]<<endl;
        }

        Activity activity(ilayer, {m.begin(), m.end()});

        auto strips = activity.make_strips();
        draw_strips(coords, strips);
        activities.push_back(activity);

        auto tot = std::accumulate(m.begin(), m.end(), 0.0);
        cerr << "Layer: " << activity.layer()
             << " activity=" << tot
             << " in: " << strips.size() << " strips"  << endl;
    }
    print();

    for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
        const auto& activity = activities[ilayer];
        cerr << "Tiling layer " << ilayer << " with " << blobs.size() << " blobs\n";
        activity.dump();
        if (blobs.empty()) {
            blobs = tiling(activity);
        }
        else {
            blobs = tiling(blobs, activity);
            if (blobs.empty()) {
                cerr << "lost m'blobs!\n";
                return -1;
            }
        }
        drop_invalid(blobs);
        dump(blobs);
        draw_points_blobs(coords, print, points, blobs);
        print();
    }

    draw_points_blobs(coords, print, points, blobs);
    for (const auto&  activity: activities) {
        auto strips = activity.make_strips();
        draw_strips(coords, strips, false);
    }
    print();

    return 0;
}
