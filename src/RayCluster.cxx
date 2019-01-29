#include "WireCellUtil/RayCluster.h"

#include <algorithm>

#include <iostream>             // debug

using namespace WireCell;

static
RayClustering::corners_t find_corners(const RayClustering::Strip& one,
                                      const RayClustering::Strip& two)
{
    RayClustering::corners_t ret;

    const auto a = one.addresses(), b = two.addresses();

    ret.push_back(std::make_pair(a.first,  b.first));
    ret.push_back(std::make_pair(a.first,  b.second));
    ret.push_back(std::make_pair(a.second, b.first));
    ret.push_back(std::make_pair(a.second, b.second));
    return ret;
}




void RayClustering::Cluster::add(const RayGrid& rg, const Strip& strip)
{
   const size_t nstrips = m_strips.size();

    if (nstrips == 0) {         // special case
        m_strips.push_back(strip);
        return;
    }

    if (nstrips == 1) {         // special case
        m_strips.push_back(strip);
        m_corners = find_corners(m_strips.front(), m_strips.back());
        return;
    }

    corners_t surviving;

    // See what old corners are inside the new strip
    for (const auto& c : m_corners) {
        double pitch = rg.pitch_location(c.first, c.second, strip.layer);

        // caution, this is probably sensitive to roundoff error.
        if (strip.bounds.first <= pitch and pitch <= strip.bounds.second) { 
            surviving.push_back(c);
        }
    }

    // see what new corners are inside all old strips;
    for (size_t si1 = 0; si1 < nstrips; ++si1) {
        auto corners = find_corners(m_strips[si1], strip);
        for (const auto& c : corners) {
            bool miss = false;
            for (size_t si2 = 0; si2 < nstrips; ++si2) {
                if (si1 == si2) { continue; }
                const auto& s2 = m_strips[si2];
                double pitch = rg.pitch_location(c.first, c.second, s2.layer);
                if (pitch < s2.bounds.first or pitch > s2.bounds.second) {
                    miss = true;
                    break;
                }
            }
            if (!miss) {
                surviving.push_back(c);
            }
        }
    }
    m_corners = surviving;
    m_strips.push_back(strip);
}

const RayClustering::corners_t& RayClustering::Cluster::corners() const
{
    return m_corners;
}



RayClustering::RayClustering(const RayGrid& rg)
    : m_rg(rg)
{
}



RayClustering::clustering_t RayClustering::cluster(const Activity& activity)
{
    auto strips = activity.make_strips();
    const size_t nstrips = strips.size();
    clustering_t ret(nstrips);
    for (size_t ind=0; ind<nstrips; ++ind) {
        ret[ind].add(m_rg, strips[ind]);
    }
    return ret;
}

RayClustering::Activity RayClustering::projection(const Cluster& cluster, const Activity& activity)
{
    const double pitch_mag = m_rg.pitch_mags()[activity.layer];

    std::vector<double> pitches;
    for (const auto& c : cluster.corners()) {
        const double p = m_rg.pitch_location(c.first, c.second, activity.layer);
        pitches.push_back(p);
    }
    if (pitches.empty()) {
        return Activity{};
    }

    const double first_pitch = pitch_mag * activity.index(activity.begin());
    const double last_pitch = pitch_mag * activity.index(activity.end()); // inclusive

    auto beg = pitches.begin();
    auto end = pitches.end();

    end = std::partition(beg, end, [&](const double& x){
            return x >= first_pitch and x <= last_pitch;});
    sort(beg, end);
    const int offset1 = *beg/pitch_mag;
    const int offset2 = *end/pitch_mag;
    return Activity{activity.layer, activity.origin,
            make_pair(activity.origin+offset1, activity.origin+offset2)};
}

RayClustering::clustering_t RayClustering::cluster(const clustering_t& prior,
                                                   const Activity& activity)
{
    clustering_t ret;

    for (const auto& clus : prior) {
        if (clus.corners().empty()) {
            std::cerr << "cluster with " << clus.strips().size() << " strips has no corners\n";
            continue;
        }
        Activity proj = projection(clus, activity);
        if (proj.empty()) {
            std::cerr << "cluster with " << clus.strips().size()
                      << " has overlap with activity in layer " << activity.layer << std::endl;
            continue;
        }
        for (auto strip : proj.make_strips()) {
            auto newclus = clus; // copy
            newclus.add(m_rg, strip);
            if (newclus.corners().empty()) {
                continue;
            }
            ret.push_back(newclus);
        }
    }

    return ret;
}

