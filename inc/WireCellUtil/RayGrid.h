/** Define a pair-wise, non-orthongal regular coordinate system.
 *
 * A ray grid is composed of a number of regular 2D "ray Cartesian
 * coordinate systems (RCCS) which may be translated and rotated
 * w.r.t. each other.  
 * 
 * Each RCCS is defined by an ordered pair of parallel rays where each
 * ray is defined in terms of its 3D Cartesian endpoints in some
 * shared global coordinate system.  The two orthogonal axes of a RCCS
 * are called its "pitch" and its "point".  The origin of the RCCS is
 * the center point of its first ray (the "pitch" ray).
 *
 * The "pitch" axis is along a mutually perpendicular line from the
 * first to the second ray.  The distance along this axis between the
 * two rays is the "pitch magnitude".
 *
 * The "point" axis is along the direction of the first ray and toward
 * its head.  No particular distance metric is defined in the point
 * direction for a single RCCS in isolation.  (But see below about
 * combining a pair of RCCSes).
 *
 * Rays in a RCCS are indexed by integer numbers.  The first ray of the
 * ordered pair has index 0 and the second ray has index 1.
 * Additional parallel rays may be considred to exist at uniformly
 * separated points on a grid along the pitch direction.
 *
 * A pair of RCCSes with differing rotations lead to intersecting
 * rays.  Their crossing points define a uniform but non-orthongonal
 * coordinate system (NOCS).  Given the uniformity, these crossing
 * points may be calculated as a function of the indices of each ray
 * in its RCCS (and simple constants defined for each RCCS).  These
 * crossing points may also be expressed in terms of the pitch
 * location of a third RCCS simply in terms of the same pair of
 * indices and constants that depend on three RCCS axes vectors.
 */

#ifndef WIRECELL_RAYGRID
#define WIRECELL_RAYGRID

#include "WireCellUtil/Point.h"
#include "WireCellUtil/ObjectArray2d.h"

#include <boost/multi_array.hpp>

#include <vector>
#include <map>

namespace WireCell {

    class RayGrid {
    public:
        // Index a ray Cartesian coordinate system
        typedef int rccs_index_t;

        // Index a point on a uniform linear grid.  Despite the
        // allowance of being signed, negative grid indices are
        // typically out of bounds.
        typedef int grid_index_t;

        // A ray may be located knowing the indices into its defining RCCS and its grid index.
        struct ray_address_t {
            rccs_index_t rccs;
            grid_index_t grid;
        };

        // Identify a pair of RCCSes
        typedef std::pair<rccs_index_t, rccs_index_t> rccs_pair_t;

        // Identify a triple of RCCSes
        typedef std::tuple<rccs_index_t, rccs_index_t, rccs_index_t> rccs_triple_t;

        // Pair of rays and a vector of pairs
        typedef std::pair<Ray,Ray> ray_pair_t;
        typedef std::vector<ray_pair_t> ray_pair_vector_t;

        // tensor type used for 3-way RCCS/NOCS coefficients 
        typedef boost::multi_array<double, 3> tensor_t;

        // A 1D array of Vectors.
        typedef std::vector<Vector> vector_array1d_t;
        // A 2D array'ish of Vectors.
        typedef ObjectArray2d<Vector> vector_array2d_t;

        // Create a ray grid by specifying the axis of projection.
        RayGrid(const ray_pair_vector_t& rays, int normal_axis=0);
        
        // Return the crossing point of the index=0 rays for two rccs.
        Vector zero_crossing(rccs_index_t one, rccs_index_t two) const;
        
        // Return the crossing point of two rays.
        Vector ray_crossing(const ray_address_t& one, const ray_address_t& two) const;

        // Return the pitch location measured in an other RCCS give of the crossing point of two rays
        double pitch_location(const ray_address_t& one, const ray_address_t& two, rccs_index_t other) const;


        int nrccs() const { return m_nrccs; }
        const std::vector<double>& pitch_mags() const { return m_pitch_mag; }
        const vector_array1d_t& pitch_dirs() const { return m_pitch_dir; }
        const vector_array1d_t& centers() const { return m_center; }

        const vector_array2d_t& ray_jumps() const { return m_ray_jump; }

    private:

        int m_nrccs;

        // Pitch magnitude for each RCCS
        std::vector<double> m_pitch_mag;

        // The unit vector in the pitch direction for each RCCS
        vector_array1d_t m_pitch_dir;

        // A point (center point) on ray 0 of each RCCS
        vector_array1d_t m_center;

        // Zero-rays crossing points indexed by RCCS index pairs.
        // Symmetric array, diagonal is invalid.
        vector_array2d_t m_zero_crossing;
    
        // Element (l,m) holds a relative vector which jumps along ray direction
        // of RCCS l between crossing points of neighboring rays of
        // RCCS m.  Not symmectric, and diagonal is invalid.
        vector_array2d_t m_ray_jump;

        // Coefficients for fast pitch location calculation.  These
        // are scalar values indexed by three different RCCS indicies.
        tensor_t m_a, m_b, m_c;

    };

}

#endif
