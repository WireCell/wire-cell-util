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
        typedef std::vector<float>::const_iterator activity_iterator;
        typedef std::pair<activity_iterator,activity_iterator> activity_range_t;
        typedef std::vector<activity_range_t> activity_ranges_t;

        // activity in one layer limited to some region in pitch
        struct Activity {
            layer_t layer;
            const activity_iterator origin;
            activity_range_t focus;
            
            activity_iterator begin() const { return focus.first; }
            activity_iterator end() const { return focus.second; }

            bool empty() const {
                return begin() == end();
            }

            // lil helpers
            
            int index(const activity_iterator& it) const {
                return it-origin;
            }
            Strip make_strip(const activity_range_t& r) const {
                return Strip{layer, std::make_pair(index(r.first), index(r.second))};
            }

            strips_t make_strips() const {
                strips_t ret;
                for (const auto& ar : active_ranges()) {
                    ret.push_back(make_strip(ar));
                }
                return ret;
            }


            activity_ranges_t active_ranges() const {
                activity_ranges_t ret;
                activity_range_t current{end(), end()};

                for (auto it = begin(); it != end(); ++it) {
                    // entering active range
                    if (current.first == end() and *it > 0.0) {
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

        };


        class Cluster {
        public:
            void add(const RayGrid& rg, const Strip& strip); 

            const strips_t& strips() const { return m_strips; }

            // Cluster corners are pair-wise ray crossing points which
            // are contained by all strips.
            const corners_t& corners() const;

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

