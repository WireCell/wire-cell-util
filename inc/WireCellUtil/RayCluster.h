/** Define a clustering of overlapping sets rays.

    Rays must satisfy the conditions given by RayGrid and terms
    defined there are used.

    A ray cluster is defined in terms of an ordered list "layers".
    Each layer is defined by a pair of parallel rays which are
    considered to bound an interval along the pitch direction of the
    layer.  A layer corresponds to an RCCS in a RayGrid.

    An cluster may be constructed with arbitrary layers however a
    cluster is considered "valid" only if the intersection of all
    layers is nonzero.  The set of pair-wise crossing points of rays
    which bound this intersection is called the cluster's perimiter.

 */

#ifndef WIRECELL_RAYCLUSTER
#define WIRECELL_RAYCLUSTER

#include <map>
#include <vector>

#include "WireCellUtil/RayGrid.h"

namespace WireCell {

    class RayClustering {
    public:
        typedef RayGrid::rccs_index_t layer_t;
        typedef RayGrid::grid_index_t index_t;
        typedef std::pair<index_t, index_t> bounds_t;

        // A corner of a cluster is identified as by the crossing
        // point of two rays.
        typedef RayGrid::ray_address_t ray_address_t;
        typedef std::pair<ray_address_t,ray_address_t> corner_t;
        typedef std::vector<corner_t> corners_t;

        struct Strip {

            // A strip is a pitch interval with a half-open bound
            // determined by the rays at the given indices.  The second
            // ray is not considered in the strip for the purpose of
            // relating to an activity vector.  The activity vector:
            //
            // |i=0,q=0|i=1,q=Q|i=2,q=0|
            //
            // gives a strip of [1,2).

            // The RCCS index this bounds is part of
            layer_t layer;

            // The pitch indices of the RCCS rays that bound this strip
            bounds_t bounds;

            corner_t addresses() const {
                return std::make_pair(ray_address_t{layer, bounds.first},
                                      ray_address_t{layer, bounds.second});
            }
        };

        typedef std::vector<Strip> strips_t;

        // Activity represents an absolutly positioned span of pitch
        // indices which may contain some measure of positive
        // activity.  The activity may be indexed by pitch index.
        class Activity {
        public:
            typedef double value_t;
            typedef std::vector<value_t> vector_t;
            typedef vector_t::const_iterator iterator_t;
            typedef std::pair<iterator_t,iterator_t> range_t;
            typedef std::vector<range_t> ranges_t;

            // Create empty activity
            Activity(layer_t layer);

            // Create an activity from a range of some vector starting
            // at given offset in the enumeration of pitch indices.
            Activity(layer_t layer, const range_t& span,
                     int offset=0, double threshold=0.0);
            
            // Produce a subspan activity between pitch indices [pi1, pi2)
            Activity subspan(int pi_begin, int pi_end) const;

            layer_t layer() const { return m_layer; }

            iterator_t begin() const;
            iterator_t end() const;

            bool empty() const;

            // lil helpers
            
            int pitch_index(const iterator_t& it) const;

            // Make a strip from a sub span of the current activity.
            Strip make_strip(const range_t& subspan) const;

            // Return strips bounding contiguous positive activity
            // subspans.
            strips_t make_strips() const;

            // Return all subspans with activities
            ranges_t active_ranges() const;

            int offset() const { return m_offset; }

        private:
            vector_t m_span;
            layer_t m_layer;
            int m_offset;
            double m_threshold;
        };


        class Cluster {
        public:
            void add(const RayGrid& rg, const Strip& strip); 

            const strips_t& strips() const { return m_strips; }

            // Cluster corners are pair-wise ray crossing points which
            // are contained by all strips.
            const corners_t& corners() const;

            bool valid() const {
                size_t nstrips = m_strips.size();
                if (nstrips == 0) { return false; } // empty
                if (nstrips == 1) { return true; }  // no corners expected
                return corners().size() > 0;
            }

        private:
            strips_t m_strips;
            corners_t m_corners;
        };

        // A clustering is a collection of clusters.
        typedef std::vector<Cluster> clustering_t;

        RayClustering(const RayGrid& rg);

        // Return a new activity which is shrunk to fall into the shadow of the cluster.
        Activity projection(const Cluster& cluster, const Activity& activity);

        // Cluster activity in one layer.
        clustering_t cluster(const Activity& activity);

        // Refine existing clusters with activity in one new layer.
        clustering_t cluster(const clustering_t& prior,
                             const Activity& activity);

    private:
        const RayGrid& m_rg;
    };
}



#endif

