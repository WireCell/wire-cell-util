#include "WireCellUtil/RayClustering.h"
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
using namespace WireCell::RayGrid;
using namespace std;

#include "raygrid.h"

void dump(const clustering_t& clusters)
{
    cerr << "-----dumping " << clusters.size() << " clusters:\n";
    for (const auto& c : clusters) {
        c.dump();
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

const int ndepos = 10;
const int neles = 10;
const double pitch_magnitude = 5;
const double gaussian = 3;
const double border = 10;
const double width = 100;
const double height = 100;

TH1F* draw_frame(TCanvas& canvas, std::string title)
{
    auto* frame = canvas.DrawFrame(-1.0*border, -1.0*border, width+border, height+border);
    frame->SetTitle(title.c_str());
    return frame;
}

const std::vector<int> layer_colors{1,1,2,3,4};

void draw_strip(const Point& head, const Point& tail,
                const Vector& raydir, int color, bool outline=true);
void draw_strip(const Point& head, const Point& tail,
                const Vector& raydir, int color, bool outline)
{
    const double shoot = 2*std::max(width,height);
    std::vector<Point> points {
        tail+raydir*shoot,
        tail-raydir*shoot,
        head-raydir*shoot,
        head+raydir*shoot
    };

    TPolyLine* pl = new TPolyLine; // like a sieve
    pl->SetLineColor(color);
    pl->SetFillColorAlpha(color, 0.1);
    for (const auto& p : points) {
        pl->SetNextPoint(p.z(), p.y());
    }
    pl->SetNextPoint(points.front().z(), points.front().y());
    pl->Draw("f");
    if (outline) {
        pl->Draw("");
    }
}

void draw_layer(Coordinates& rg, int ilayer,
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
        draw_strip(tail, head, raydir, color);
    }
}


void draw_strips(Coordinates& rg, const strips_t& strips, bool outline=true);
void draw_strips(Coordinates& rg, const strips_t& strips, bool outline)
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
        draw_strip(tail, head, raydir, color, outline);
    }
}


void draw_cluster(Coordinates& rg, const Cluster& clus, int color=1)
{
    const auto& corners = clus.corners();
    if (corners.empty()) {
        return;
    }

    std::vector<Point> points;
    Point center;
    for (const auto& corn : corners) {
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
        : canvas("test_rayclustering", "Ray Cluster", 500, 500)
        , fname(fn) { canvas.Print((fname + ".pdf[").c_str(), "pdf"); }
    ~Printer() { canvas.Print((fname+".pdf]").c_str(), "pdf"); }
    void operator()() { canvas.Print((fname+".pdf").c_str(), "pdf"); }
};


void draw_points_clusters(Coordinates& rg, Printer& print,
                          const std::vector<Point>& points,
                          const clustering_t& clusters)
{
    int nstrips = 0;
    for (const auto& c : clusters) {
        nstrips += c.strips().size();
    }

    draw_frame(print.canvas, Form("%d points, %d clusters, %d strips",
                                  (int)points.size(), (int)clusters.size(), nstrips));
    for (size_t ipt=0; ipt<points.size(); ++ipt ) {
        const auto& p = points[ipt];
        draw_point(p, 1, 24, ipt+1);
    }
    for (size_t ic = 0; ic<clusters.size(); ++ic) {
        draw_cluster(rg, clusters[ic],1);
    }
}


int main(int argc, char* argv[])
{
    Printer print(argv[0]);

    auto raypairs = make_raypairs(width, height, pitch_magnitude);
    const int nlayers = raypairs.size();

    Coordinates rg(raypairs);

    const auto& pitches = rg.pitch_dirs();
    const auto& centers = rg.centers();
    const auto& pitch_mags = rg.pitch_mags();

    Clustering rc(rg);

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
        draw_layer(rg, ilayer, pitch_mags[ilayer],
                   pitches[ilayer], centers[ilayer], measures[ilayer]);
    }
    print();
        
    clustering_t clusters;

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
        draw_strips(rg, strips);
        activities.push_back(activity);

        auto tot = std::accumulate(m.begin(), m.end(), 0.0);
        cerr << "Layer: " << activity.layer()
             << " activity=" << tot
             << " in: " << strips.size() << " strips"  << endl;
    }
    print();

    for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
        const auto& activity = activities[ilayer];
        cerr << "Clustering layer " << ilayer << " with " << clusters.size() << " clusters\n";
        activity.dump();
        if (clusters.empty()) {
            clusters = rc.cluster(activity);
        }
        else {
            clusters = rc.cluster(clusters, activity);
            if (clusters.empty()) {
                cerr << "lost m'clusters!\n";
                return -1;
            }
        }
        drop_invalid(clusters);
        dump(clusters);
        draw_points_clusters(rg, print, points, clusters);
        print();
    }

    draw_points_clusters(rg, print, points, clusters);
    for (const auto&  activity: activities) {
        auto strips = activity.make_strips();
        draw_strips(rg, strips, false);
    }
    print();

    return 0;
}
