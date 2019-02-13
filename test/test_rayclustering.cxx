#include "WireCellUtil/RayClustering.h"
#include "WireCellUtil/RayTiling.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"


#include <math.h> 

#include <random>
#include <fstream>
#include <string>
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
#include "raygrid_dump.h"

std::vector<Point> make_points(std::default_random_engine& generator, double x)
{
    std::vector<Point> points;
    std::uniform_real_distribution<double> position(0,std::max(width,height));
    std::normal_distribution<double> spread(0.0, gaussian);
    for (int idepo=0;idepo<ndepos;++idepo) {
        Point cp(0, position(generator), position(generator));
        for (int iele=0; iele<neles; ++iele) {
            const Point delta(x, spread(generator), spread(generator));
            const Point pt = cp + delta;
            if (pt.y() < -border or pt.y() > height+border or
                pt.z() < -border or pt.z() > width+border) {
                std::cerr << "Rejecting far away point: " << cp << " + " << delta << std::endl;
                continue;
            }
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
            if (ilayer <= 1) {
                if (pit_ind >= 1 or pit_ind < 0) {
                    std::cerr << "mm: pit_ind=" << pit_ind << " with ipt=" << ipt << "\n";
                    if (pit_ind == 1) {
                        std::cerr << "\tpit=" << pit << " cen=" << cen << " rel=" << rel <<"\n";
                    }
                    continue;
                }
            }
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
        std::cerr << "Make activity for layer: " << ilayer << ": " << m.size() << " " << std::endl;
        Activity activity(ilayer, {m.begin(), m.end()});
        Assert(!activity.empty());
        activities.push_back(activity);
    }
    return activities;
}

struct Chirp {
    const blobs_t& one;
    const blobs_t& two;
    Coordinates& coords;

    typedef typename std::unordered_set<std::size_t> indices_t;
    indices_t *sel1;
    indices_t *sel2;

    Chirp(const blobs_t& one, const blobs_t& two,
          Coordinates& coords, JsonEvent& dumper)
        : one(one)
        , two(two)
        , coords(coords)
        , sel1(new indices_t)
        , sel2(new indices_t)
        {}

    bool in(const blobref_t& a, const blobref_t& b) {
        if (surrounding(a, b)) {
            cerr << "surrounding\n";
            return true;
        }

        const auto& astrips = a->strips();
        //const auto& bstrips = b->strips();
        const int nlayers = astrips.size();

        // if at least one corner of b is in side a, return true
        for (const auto& c : b->corners()) {
            int found = 0;
            // go through each layer of blob a
            for (layer_index_t layer = 0; layer < nlayers; ++layer) {
                const auto& astrip = astrips[layer];
                if (layer == c.first.layer) {
                    cerr << "L" << layer << " A: " << astrip << " " << c << endl;
                    if (astrip.on(c.first.grid)) {
                        cerr << "\ton with found="<<found<<" nlayers="<<nlayers<<"\n";
                        ++found;
                        continue;
                    }
                    cerr << "\toff with found="<<found<<" nlayers="<<nlayers<<"\n";
                    break;
                }
                if (layer == c.second.layer) {
                    cerr << "L" << layer << " A: " << astrip << " " << c << endl;
                    if (astrip.on(c.second.grid)) {
                        cerr << "\ton with found="<<found<<" nlayers="<<nlayers<<"\n";
                        ++found;
                        continue;
                    }
                    cerr << "\toff with found="<<found<<" nlayers="<<nlayers<<"\n";
                    break;
                }
                const double ploc = coords.pitch_location(c.first, c.second, layer);
                const int pind = coords.pitch_index(ploc, layer);

                cerr << "L" << layer << " A: " << astrip << " pind=" << pind << " ploc=" << ploc
                     << " " << c << endl;
                if (astrip.in(pind)) {
                    cerr << "\tin with found="<<found<<" nlayers="<<nlayers<<"\n";
                    ++found;
                }
                else {
                    cerr << "\tout with found="<<found<<" nlayers="<<nlayers<<"\n";
                    break;
                }
            }
            if (found == nlayers) {
                return true;
            }
        }
        return false;
    }

    void operator()(const blobref_t& a, const blobref_t& b) {


        const std::size_t d1 = a-one.begin();
        const std::size_t d2 = b-two.begin();

        cerr << "overlap: a" << d1 << " and b" << d2 << "\n";
        cerr << "\tblob a #" << d1 << ": ";
        a->dump();
        cerr << "\tblob b #" << d2 << ": ";
        b->dump();

        if (!this->in(a,b)) {
            cerr << "NO CONTAINED CORNERS\n";
            //Assert(this->in(a,b));
        }

        sel1->insert(d1);
        sel2->insert(d2);

    }

    void dump(JsonEvent& dumper) {
        int number = 0;
        for (const auto ind : *sel1) {
            const auto& br = one[ind];
            dumper(br, 10.0, 1.0, 1, ind);
            ++number;
        }
        number = 0;
        for (const auto ind : *sel2) {
            const auto& br = two[ind];
            dumper(br, 20.0, 1.0, 2, ind);
            ++number;
        }

        return;
    }
};
    
void test_blobs(const blobs_t& blobs)
{
    for (const auto& blob : blobs) {
        const auto& strips = blob.strips();
        Assert(strips[0].bounds.first == 0);
        Assert(strips[0].bounds.second == 1);
        Assert(strips[1].bounds.first == 0);
        Assert(strips[1].bounds.second == 1);
    }
}


int main(int argc, char* argv[])
{
    Printer print(argv[0]);

    auto raypairs = make_raypairs(width, height, pitch_magnitude);

    Coordinates coords(raypairs);

    Tiling tiling(coords);

    std::default_random_engine generator;
    auto pts1 = make_points(generator, 10.0);
    auto pts2 = make_points(generator, 20.0);

    auto meas1 = make_measures(coords, pts1);
    auto meas2 = make_measures(coords, pts2);

    auto act1 = make_activities(coords, meas1);
    auto act2 = make_activities(coords, meas2);

    auto blobs1 = make_blobs(coords, act1);
    auto blobs2 = make_blobs(coords, act2);

    test_blobs(blobs1);
    test_blobs(blobs2);       

    // fixme: blobs are missing these features:
    // - sort corners
    // - apply X offset
    // - assign some value
    // this is maybe best done converting from Blob to another rep

    JsonEvent dumper(coords);
    for (const auto& pt : pts1) { dumper(pt); }
    for (const auto& pt : pts2) { dumper(pt); }
    Chirp chirp(blobs1, blobs2, coords, dumper);
    associator_t chirpf = chirp;
    associate(blobs1, blobs2, chirpf);

    chirp.dump(dumper);

    std::string fout = argv[0];
    fout += ".json";
    dumper.dump(fout);

    return 0;
}

