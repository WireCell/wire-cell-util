#include "WireCellUtil/RayCluster.h"

#include <algorithm>

#include <iostream>             // debug

using namespace WireCell;

RayClustering::Activity::Activity(layer_t layer)
    : m_span()
    , m_layer(layer)
    , m_offset(0)
    , m_threshold(0)
{
}
RayClustering::Activity::Activity(layer_t layer, const range_t& span, int offset,
                                  double threshold)
    : m_span()
    , m_layer(layer)
    , m_offset(offset)
    , m_threshold(threshold)
{
    iterator_t b = span.first;
    while (*b <= m_threshold and b != span.second) {
        ++b;
        ++m_offset;
    }
    iterator_t e = span.second;
    while (e != b and *(e-1) <= m_threshold) {
        --e;
    }
    m_span.insert(m_span.begin(), b,e);
}

RayClustering::Activity::iterator_t RayClustering::Activity::begin() const
{
    return m_span.begin();
}

RayClustering::Activity::iterator_t RayClustering::Activity::end() const
{
    return m_span.end();
}

bool RayClustering::Activity::empty() const 
{
    return m_span.empty();
}

int RayClustering::Activity::pitch_index(const iterator_t& it) const
{
    return m_offset + it-m_span.begin();
}
// Produce a subspan activity between pitch indices [pi1, pi2)
RayClustering::Activity RayClustering::Activity::subspan(int pi_begin, int pi_end) const
{
    const int beg = pi_begin-m_offset;
    const int end = pi_end-m_offset;
    if (beg < m_offset or beg > end or beg-end >= (int)m_span.size()) {
        std::cerr
            << "activity::subspan bogus pi_begin="<<pi_begin<<" pi_end="<<pi_end
            << " m_offset="<<m_offset << " span.size=" << m_span.size()
            << std::endl;
        return Activity(m_layer);
    }
    return Activity(m_layer, {begin()+beg, begin()+end}, pi_begin);
}


RayClustering::Strip
RayClustering::Activity::make_strip(const RayClustering::Activity::range_t& r) const
{
    return RayClustering::Strip{m_layer,
            std::make_pair(pitch_index(r.first),
                           pitch_index(r.second))};
}

RayClustering::strips_t RayClustering::Activity::make_strips() const 
{
    strips_t ret;
    for (const auto& ar : active_ranges()) {
        ret.push_back(make_strip(ar));
    }
    return ret;
}


RayClustering::Activity::ranges_t RayClustering::Activity::active_ranges() const
{
    ranges_t ret;
    range_t current{end(), end()};

    for (auto it = begin(); it != end(); ++it) {
        // entering active range
        if (current.first == end() and *it > m_threshold) {
            current.first = it;
            continue;
        }
        // exiting active range
        if (current.first != end() and *it <= 0.0) {
            current.second = it;
            ret.push_back(current);
            current.first = end();
        }
    }
    if (current.first != end()) {
        current.second = end();
        ret.push_back(current);
    }
    return ret;
}

/*********************************/

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
    // special case.  A single layer cluster effectively extends to
    // infinity along the ray direction so any possible activity
    // projects.
    if (cluster.strips().size() == 1) {
        return activity;
    }

    const double pitch_mag = m_rg.pitch_mags()[activity.layer()];

    std::vector<double> pitches;
    const auto corners = cluster.corners();
    if (corners.empty()) {
        std::cerr << "projection: got empty cluster\n";
        return Activity(activity.layer());
    }
    for (const auto& c : cluster.corners()) {
        const double p = m_rg.pitch_location(c.first, c.second, activity.layer());
        pitches.push_back(p);
        std::cerr << "cluster corner: L" << activity.layer()
                  << " [{" << c.first.rccs << "," << c.first.grid << "},"
                  << "{" << c.second.rccs << "," << c.second.grid << "}] p=" << p << "\n";
    }
    if (pitches.empty()) {
        std::cerr << "projection: got not pitches\n";
        return Activity(activity.layer());
    }

    const double first_pitch = pitch_mag * activity.pitch_index(activity.begin());
    const double last_pitch = pitch_mag * activity.pitch_index(activity.end()); // inclusive

    auto pbeg = pitches.begin();
    auto pend = pitches.end();

    std::cerr << "Checking activity projection over "
              << activity.end() - activity.begin() << " activity bins:\n";
    std::cerr << "\tactivity pitches: [" << first_pitch << "," << last_pitch <<"]\n";
    std::cerr << "\tcluster pitches: ";
    for (auto pit=pbeg; pit!=pend; ++pit) {
        std::cerr << " " << *pit;
    }
    std::cerr << "\n";
    pend = std::partition(pbeg, pend, [&](const double& x){
            return x >= first_pitch and x <= last_pitch;});
    const auto mm = std::minmax_element(pbeg, pend);
    const int offset1 = std::floor((*mm.first)/pitch_mag);
    const int offset2 = std::ceil((*mm.second)/pitch_mag);
    return activity.subspan(offset1, offset2);
}


RayClustering::clustering_t RayClustering::cluster(const clustering_t& prior,
                                                   const Activity& activity)
{
    clustering_t ret;

    for (const auto& clus : prior) {
        size_t nstrips = clus.strips().size();
        Activity proj = projection(clus, activity);
        if (proj.empty()) {
            std::cerr << "cluster with " << nstrips
                      << " strips has no overlap with activity in layer "
                      << activity.layer() << std::endl;
            continue;
        }
        auto strips = proj.make_strips();
        for (auto strip : strips) {
            std::cerr << "\tstrip: L" << strip.layer << " [" << strip.bounds.first << "," << strip.bounds.second << "]\n";
            Cluster newclus = clus; // copy
            newclus.add(m_rg, strip);
            if (newclus.corners().empty()) {
                continue;
            }
            ret.push_back(newclus);
        }
    }

    return ret;
}

