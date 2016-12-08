#ifndef WIRECELLUTIL_PIMPOS
#define WIRECELLUTIL_PIMPOS

#include "WireCellUtil/Point.h"
#include "WireCellUtil/Units.h"

namespace WireCell {

    /** \brief Pitch-Impact-Position.

	A Pimpos object encapsulates information about a continuous
	and discrete coordinate system associated with a wire plane.

	The coordinate system has three orthogonal axes:

	0) anti nominal drift direction (away from the wire plane) and
	1) along the wire direction such that,
	2) the pitch direction is the cross product of axis0 X axis1.

	In the pitch direction, two related and discrete scales are
	supported.  The wire pitch and a subdivision into "impact
	positions".  There are a given number of impact positions
	spanning 1/2 of the wire pitch.  The first impact position may
	either be centered on the wire or offset by 1/2 the distance
	between impact positions.  If centered then there will be two
	impact positions at the mid point between two wires, each
	infinitesimally closer to its associated wire.

    */

    class Pimpos {
    public:
	/** Create a Pimpos object for a particular plane.
	    
	    \param origin is a Point which sets an origin.  This is
	    usually the center of wire 0.  

	    \param wire is a Vector which sets the direction of the
	    wires in the plane.  

	    \param pitch is a Vector which sets the direction and
	    magnitude of the pitch of the wires in the plane.
	    Nominal anti-drift direction is thus (wire X pitch).

	    \param nimpacts gives the number of impact positions for
	    one half of the pitch.

	    \param impact_offset gives a spatial shift in the pitch
	    direction.  This shift is not interpreted.  If a very
	    large shift is given a particular impact which is
	    associated to a particular wire may actually be nearest to
	    a totally different wire.
	    

	    Any quantities must be expressed in the WCT system of units.
	 */
	Pimpos(const Point& origin, const Vector& wire, const Vector& pitch,
	       int nimpacts_per_wire=10, double impact_offset=0.0*units::m);

	const Point& origin() const { return m_origin; }
	const Vector& axis(int i) const { return m_axis[i]; }

	/// Return the vector from the origin to the given point.
	Vector relative(const Point& pt) const;

	/// Return the distance from origin to point along the given
	/// axis.
	double distance(const Point& pt, int axis) const;

	/// Transform the given point into the Pimpos coordinate
	/// system.
	Point transform(const Point& pt) const;
	
	/// Return the absolute impact number given the pitch distance;
	int impact(double pitch_dist) const;

	/// Return the pitch distance at the absolute impact number.
	double pitch(int absimp) const;

	/// Return the absolute impact number from the wire number and
	/// an impact number counting relative to that wire.
	int absolute_impact(int wire, int relimp) const;

	/// Return the wire and relative impact associated with the
	/// absolute impact number.
	std::pair<int,int> wire_impact(int absimp) const;

	/// Return the (wire,relimp) which is corresponds to the
	/// impact postion relimp relative to the wire but on the
	/// other side of the wire.
	std::pair<int,int> reflect(int wire, int relimp) const;

    private:
	Point m_origin;
	Vector m_axis[3];
	double m_pitch;
	double m_impdist;
	double m_impoff;
	int m_niperw;
    };

}


#endif
