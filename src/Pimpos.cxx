#include "WireCellUtil/Pimpos.h"

using namespace WireCell;



Pimpos::Pimpos(const Point& origin, const Vector& wire, const Vector& pitch,
	       int nimpacts_per_wire, double impact_offset)
    : m_origin(origin)
    , m_axis{Vector(0,0,0), wire.norm(), pitch.norm()}
    , m_pitch(pitch.magnitude())
    , m_impdist(pitch.magnitude()/nimpacts_per_wire)
    , m_impoff(impact_offset)
    , m_niperw(nimpacts_per_wire)
{
    // drift = wire X pitch 
    m_axis[0] = m_axis[1].cross(m_axis[2]);
}


Vector Pimpos::relative(const Point& pt) const
{
    return pt - m_origin;
}

double Pimpos::distance(const Point& pt, int axis) const
{
    return m_axis[axis].dot(relative(pt));
}

Point Pimpos::transform(const Point& pt) const
{
    const Vector v = relative(pt);
    Point ret;
    for (int axis=0; axis<3; ++axis) {
	ret[axis] = m_axis[axis].dot(v);
    }
    return ret;
}

int Pimpos::impact(double pitch_dist) const
{
    return round((pitch_dist - m_impoff)/m_impdist);
}

double Pimpos::pitch(int absimp) const
{
    return absimp * m_impdist + m_impoff;
}

int Pimpos::absolute_impact(int wire, int impact) const
{
    return wire * m_niperw + impact;
}

	/// Return the wire and relative impact associated with the
	/// absolute impact number.
std::pair<int,int> Pimpos::wire_impact(int absimp) const
{
    const int iwire = int(absimp + 0.5*m_niperw) / m_niperw; // integer division

    const int iimp = absimp - (iwire*m_niperw);
    return std::make_pair(iwire,iimp);
}

std::pair<int,int> Pimpos::reflect(int wire, int relimp) const
{
    const int me = absolute_impact(wire,0);
    const int you = absolute_impact(wire, relimp);
    const int diff = you-me;
    return wire_impact(me - diff);
}


// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
