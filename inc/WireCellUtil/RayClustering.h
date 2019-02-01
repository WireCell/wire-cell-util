/** Define a clustering of overlapping sets rays.

    This is part of Ray Grid.

    See the WCT manual for details.
 */

#ifndef WIRECELL_RAYCLUSTERING
#define WIRECELL_RAYCLUSTERING

#include <map>
#include <vector>
#include <iostream>

#include "WireCellUtil/RayGrid.h"

namespace WireCell {

    namespace RayGrid {

        struct Strip {

            // The layer index this stripo is in
            layer_index_t layer;

            // The pitch indices bounding the strip in its layer pitch direction.
            grid_range_t bounds;

            crossing_t addresses() const {
                return std::make_pair(coordinate_t{layer, bounds.first},
                                      coordinate_t{layer, bounds.second});
            }

            bool in(grid_index_t pitch_index) const {
                return bounds.first <= pitch_index and pitch_index < bounds.second;
            }
                
        };
        typedef std::vector<Strip> strips_t;

        // Activity represents an absolutly positioned span of pitch
        // indices which may contain some measure of some kind of
        // activity (ie, an ADC sum measured in the channels fed by
        // wires represented by the rays).  The activity may be
        // indexed by pitch index.  Each index n is considered to
        // contain the activity from its pitch up to but not including
        // the pitch of the n+1 bin.
        class Activity {
        public:
            typedef double value_t;
            typedef std::vector<value_t> vector_t;
            typedef vector_t::const_iterator iterator_t;
            typedef std::pair<iterator_t,iterator_t> range_t;
            typedef std::vector<range_t> ranges_t;

            // Create empty activity
            Activity(layer_index_t layer);

            // Create an activity from a range of some vector starting
            // at given offset in the enumeration of pitch indices.
            Activity(layer_index_t layer, const range_t& span,
                     int offset=0, double threshold=0.0);
            
            // Produce a subspan activity between pitch indices [pi1, pi2)
            Activity subspan(int pi_begin, int pi_end) const;

            layer_index_t layer() const { return m_layer; }

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
            void dump() const;

        private:
            vector_t m_span;
            layer_index_t m_layer;
            int m_offset;
            double m_threshold;
        };
        typedef std::vector<Activity> activities_t;

        class Cluster {
        public:
            void add(const Coordinates& rg, const Strip& strip); 

            const strips_t& strips() const { return m_strips; }

            // Cluster corners are pair-wise ray crossing points which
            // are contained by all strips.
            const crossings_t& corners() const;

            bool valid() const {
                size_t nstrips = m_strips.size();
                if (nstrips == 0) { return false; } // empty
                if (nstrips == 1) { return true; }  // no corners expected
                return corners().size() > 0;
            }

            void dump() const;

        private:
            strips_t m_strips;
            crossings_t m_corners;
        };


        // A clustering is a collection of clusters.
        typedef std::vector<Cluster> clustering_t;

        class Clustering {
        public:

            Clustering(const Coordinates& rg);

            // Return a new activity which is shrunk to fall into the shadow of the cluster.
            Activity projection(const Cluster& cluster, const Activity& activity);

            // Cluster activity in one layer.
            clustering_t cluster(const Activity& activity);

            // Refine existing clusters with activity in one new layer.
            clustering_t cluster(const clustering_t& prior,
                                 const Activity& activity);

        private:
            const Coordinates& m_rg;
        };


        /// free functions

        // Remove any invalid clusers, return number removed.
        size_t drop_invalid(clustering_t& clusters);

        // One stop shopping to generate clusters from activity
        clustering_t cluster(const Coordinates& rg, const activities_t& activities);


        inline
        std::ostream& operator<<(std::ostream& os, const WireCell::RayGrid::Strip& s)
        {
            os << "<strip L" << s.layer << " pind:["  << s.bounds.first << "," << s.bounds.second << "]>";
            return os;
        }

        inline
        std::ostream& operator<<(std::ostream& os, const WireCell::RayGrid::Activity& a)
        {
            int b = a.pitch_index(a.begin()), e = a.pitch_index(a.end());
            auto strips = a.make_strips();
            os << "<activity L" << a.layer() << " " << strips.size() << " strips over pind:["  << b << "," << e << "]>";
            return os;
        }

        inline
        std::ostream& operator<<(std::ostream& os, const WireCell::RayGrid::Cluster& c)
        {
            os << "<cluster " << c.strips().size() << " strips, " << c.corners().size() << " corners>";
            return os;
        }

        inline
        std::ostream& operator<<(std::ostream& os, const WireCell::RayGrid::crossing_t& c)
        {
            os << "<corner [{L" << c.first.layer << ",G" << c.first.grid << "},"
               << "{L" << c.second.layer << ",G" << c.second.grid << "}]>";
            return os;
        }
    }
} // WireCell namespace

#endif

