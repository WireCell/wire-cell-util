#include "WireCellUtil/Point.h"

#include <algorithm> // minmax

namespace instantiations {

    WireCell::Point a_point;
    WireCell::Ray a_ray;
    WireCell::PointVector a_point_vector;
    WireCell::PointValue a_point_value;
    WireCell::PointValueVector a_point_value_vector;
    WireCell::PointF a_sad_little_point;


}

std::ostream& operator<<(std::ostream& os, const WireCell::Ray& ray)
{
    os << "["  << ray.first << " --> " << ray.second << "]";
    return os;
}


bool WireCell::ComparePoints::operator()(const WireCell::Point& lhs,
					 const WireCell::Point& rhs)
{
    double mag = (lhs-rhs).magnitude();
    if (mag < 1e-10) {
	return false;		// consider them equal
    }
    // otherwise, order them by x,y,z
    for (int ind=0; ind<3; ++ind) {
	if (lhs[ind] < rhs[ind]) {
	    return true;
	}
    }
    return false;
}

bool WireCell::point_contained(const Point& point, const Ray& bounds)
{
    for (int axis = 0; axis<3; ++axis) {
	if (!point_contained(point, bounds, axis)) {
	    return false;
	}
    }
    return true;
}

bool WireCell::point_contained(const Point& point, const Ray& bounds, int axis)
{
    std::pair<double, double> mm = std::minmax(bounds.first[axis], bounds.second[axis]);
    return mm.first <= point[axis] && point[axis] <= mm.second;
}

