#include "WireCellUtil/RayClustering.h"
#include "WireCellUtil/RayTiling.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"


#include <math.h> 

#include <random>
#include <iostream>

using namespace WireCell;
using namespace WireCell::Waveform;
using namespace WireCell::RayGrid;
using namespace std;


const int ndepos = 10;
const int neles = 10;
const double pitch_magnitude = 5;
const double gaussian = 3;
const double border = 10;
const double width = 100;
const double height = 100;

// local helper codes
#include "raygrid.h"
#include "raygrid_draw.h"

std::vector<Point> make_points(std::default_random_engine& generator)
{
    std::vector<Point> points;
    std::uniform_real_distribution<double> position(0,std::max(width,height));
    std::normal_distribution<double> spread(0.0, gaussian);
    for (int idepo=0;idepo<ndepos;++idepo) {
        Point cp(0, position(generator), position(generator));
        for (int iele=0; iele<neles; ++iele) {
            Point delta(0, spread(generator), spread(generator));
            points.push_back(cp+delta);
        }
    }
    return points;
}


typedef std::vector<Activity::value_t> measure_t;
std::vector<measure_t> make_measures(Coordinates& coords, const std::vector<Point>& points)
{
    int nlayers = coords.nlayers();
    std::vector<measure_t> measures(nlayers);
    const auto& pitches = coords.pitch_dirs();
    const auto& centers = coords.centers();
    const auto& pitch_mags = coords.pitch_mags();

    for (size_t ipt=0; ipt<points.size(); ++ipt ) {
        const auto& p = points[ipt];
        for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
            const auto& pit = pitches[ilayer];
            const auto& cen = centers[ilayer];
            const auto rel = p-cen;
            const int pit_ind = pit.dot(rel)/pitch_mags[ilayer]; 
            auto& m = measures[ilayer];
            if ((int)m.size() <= pit_ind) {
                m.resize(pit_ind+1, 0.0);
            }
            m[pit_ind] += 1.0;
        }
    }

    return measures;
}

activities_t make_activities(Coordinates& coords, std::vector<measure_t>& measures)
{
    int nlayers = coords.nlayers();
    activities_t activities;
    for (int ilayer = 0; ilayer<nlayers; ++ilayer) {
        auto& m = measures[ilayer];
        std::cerr << ilayer << ": " << m.size() << std::endl;
        Activity activity(ilayer, {m.begin(), m.end()});
        Assert(!activity.empty());
        activities.push_back(activity);
    }
    return activities;
}

struct Chirp {
    const blobs_t& one;
    const blobs_t& two;
    Chirp(const blobs_t& one, const blobs_t& two) : one(one), two(two) {}

    void operator()(blobref_t& a, blobref_t& b) const{
        cerr << a-one.begin() << " " << b-two.begin() << "\n";
    }
};
    


int main(int argc, char* argv[])
{
    Printer print(argv[0]);

    auto raypairs = make_raypairs(width, height, pitch_magnitude);

    Coordinates coords(raypairs);

    Tiling tiling(coords);

    std::default_random_engine generator;
    auto pts1 = make_points(generator);
    auto pts2 = make_points(generator);

    auto meas1 = make_measures(coords, pts1);
    auto meas2 = make_measures(coords, pts2);

    auto act1 = make_activities(coords, meas1);
    auto act2 = make_activities(coords, meas2);

    auto blobs1 = make_blobs(coords, act1);
    auto blobs2 = make_blobs(coords, act2);

    associator_t chirp = Chirp(blobs1, blobs2);
    associate(blobs1, blobs2, chirp);

    return 0;
}

