#include "WireCellUtil/RayCluster.h"
#include "WireCellUtil/Waveform.h"

#include "TCanvas.h"
#include "TMarker.h"
#include "TText.h"
#include "TLine.h"
#include "TPolyLine.h"
#include "TArrow.h"
#include "TH1F.h"

#include <math.h> 

#include <random>
#include <iostream>

using namespace WireCell;
using namespace WireCell::Waveform;
using namespace std;

#include "raygrid.h"

void dump(const RayClustering::clustering_t& clusters)
{
    cerr << "-----dumping " << clusters.size() << " clusters:\n";
    for (const auto& c : clusters) {
        const auto& strips = c.strips();
        cerr << "\tvalid:" << c.valid() << ", " << strips.size() << " strips:";
        for (const auto& s : strips) {
            cerr << " L" << s.layer << " [" << s.bounds.first << "," << s.bounds.second << "]";
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
    cerr << "------\n";
}

void draw_point(const Point& p, float size=1, int style=20, int color=1);
void draw_point(const Point& p, float size, int style, int color)
{
    TMarker m;
    m.SetMarkerColor(color);
    m.SetMarkerSize(size);
    m.SetMarkerStyle(style);
    m.DrawMarker(p.z(), p.y());
}
void draw_ray(const Ray& ray, int color=1)
{
    TArrow l;
    l.SetLineColor(color);
    l.DrawLine(ray.first.z(), ray.first.y(),
               ray.second.z(), ray.second.y());
}

TH1F* draw_frame(TCanvas& canvas, std::string title)
{
    auto* frame = canvas.DrawFrame(-10,-10,110,110);
    frame->SetTitle(title.c_str());
    return frame;
}

const std::vector<int> layer_colors{1,1,2,3,4};

void draw_layer(RayGrid& rg, int ilayer,
                double pitch_mag,
                const Point& pitch,
                const Point& center,
                const std::vector<double>& measure)
{
    const Vector ecks(1,0,0);
    const auto raydir = ecks.cross(pitch);


    for (size_t ind=0; ind<measure.size(); ++ind) {
        int color = layer_colors[ilayer];
        if (measure[ind] <= 0.0) { continue; }
        const auto tail = center + ind*pitch_mag*pitch;
        const auto head = center + (ind+1)*pitch_mag*pitch;
        draw_ray(Ray(tail, head), color);
        draw_ray(Ray(tail, tail+raydir*pitch_mag), color);
        draw_ray(Ray(head, head+raydir*pitch_mag), color);
    }
}

void draw_strips(RayGrid& rg, const RayClustering::strips_t& strips)
{
    const Vector ecks(1,0,0);

    for (const auto& strip : strips) {
        int color = layer_colors[strip.layer];
        const auto& pitch = rg.pitch_dirs()[strip.layer];
        const auto raydir = ecks.cross(pitch);
        const auto& center = rg.centers()[strip.layer];

        const double pitch_mag = rg.pitch_mags()[strip.layer];

        const auto pind1 = strip.bounds.first;
        const auto pind2 = strip.bounds.second;
        const double pitch_dist = std::abs(pind1-pind2)*pitch_mag;

        const auto tail = center + pind1*pitch_mag*pitch;
        const auto head = tail + pitch_dist*pitch;
        draw_ray(Ray(tail, head), color);
        draw_ray(Ray(tail, tail+raydir*5.0), color);
        draw_ray(Ray(head, head+raydir*5.0), color);

    }
}


void draw_cluster(RayGrid& rg, const RayClustering::Cluster& clus, int color=1)
{
    std::vector<Point> points;
    Point center;
    for (const auto& corn : clus.corners()) {
        const auto p = rg.ray_crossing(corn.first, corn.second);
        center += p;
        points.push_back(p);
    }
    center = center * (1.0/points.size());
    sort(points.begin(), points.end(),
         [&](const Point& a, const Point&b) {
             const Point ac = a-center;
             const Point bc = b-center;
             const double anga = atan2(ac.y(), ac.z());
             const double angb = atan2(bc.y(), bc.z());
             return anga > angb;
         });

    TPolyLine* pl = new TPolyLine; // like a sieve
    pl->SetLineColor(color);
    for (const auto& p : points) {
        pl->SetNextPoint(p.z(), p.y());
    }
    pl->SetNextPoint(points.front().z(), points.front().y());
    pl->Draw();
}



struct Printer {
    TCanvas canvas;
    std::string fname;
    Printer(std::string fn)
        : canvas("test_raycluster", "Ray Cluster", 500, 500)
        , fname(fn) { canvas.Print((fname + ".pdf[").c_str(), "pdf"); }
    ~Printer() { canvas.Print((fname+".pdf]").c_str(), "pdf"); }
    void operator()() { canvas.Print((fname+".pdf").c_str(), "pdf"); }
};

int main(int argc, char* argv[])
{
    Printer print(argv[0]);

    const double width=100, height=100, pitch_mag = 5;
    auto raypairs = make_raypairs(width, height, pitch_mag);
    const int nlayers = raypairs.size();

    RayGrid rg(raypairs);

    const auto& pitches = rg.pitch_dirs();
    const auto& centers = rg.centers();
    const auto& pitch_mags = rg.pitch_mags();

    RayClustering rc(rg);

    std::vector< std::vector<RayClustering::Activity::value_t> > measures(nlayers);

    std::default_random_engine generator;
    std::uniform_real_distribution<double> position(0,100);
    std::normal_distribution<double> spread(0.0, 1.0);
    const int ndepos = 10;
    const int neles = 10;
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
        draw_layer(rg, ilayer, pitch_mags[ilayer],
                   pitches[ilayer], centers[ilayer], measures[ilayer]);
    }
    print();
        
    RayClustering::clustering_t clusters;

    draw_frame(print.canvas, "Points and Strips");
    for (size_t ipt=0; ipt<points.size(); ++ipt ) {
        const auto& p = points[ipt];
        draw_point(p, 1, 24, ipt+1);
    }
    std::vector<RayClustering::Activity> activities;
    for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
        auto& m = measures[ilayer];

        for (size_t ind=0; ind<m.size(); ++ind) {
            if (m[ind] <= 0.0) { continue; }
            std::cerr << ilayer << "[" << ind << "]=" << m[ind]<<endl;
        }

        RayClustering::Activity activity(ilayer, {m.begin(), m.end()});

        auto strips = activity.make_strips();
        draw_strips(rg, strips);
        activities.push_back(activity);

        auto tot = std::accumulate(m.begin(), m.end(), 0.0);
        cerr << "Layer: " << activity.layer()
             << " activity=" << tot
             << " in: " << strips.size() << " strips"  << endl;
    }
    print();

    for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
        cerr << "Clustering layer " << ilayer << endl;
        const auto& activity = activities[ilayer];
        if (clusters.empty()) {
            clusters = rc.cluster(activity);
        }
        else {
            clusters = rc.cluster(clusters, activity);
        }
        dump(clusters);
    }
    draw_frame(print.canvas, "Points and Cluster");
    for (size_t ipt=0; ipt<points.size(); ++ipt ) {
        const auto& p = points[ipt];
        draw_point(p, 1, 24, ipt+1);
    }
    
    for (size_t ic = 0; ic<clusters.size(); ++ic) {
        draw_cluster(rg, clusters[ic],ic+1);
    }
    print();

    return 0;
}
