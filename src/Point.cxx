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

bool WireCell::point_contained(const WireCell::Point& point, const WireCell::Ray& bounds)
{
    for (int axis = 0; axis<3; ++axis) {
	if (!point_contained(point, bounds, axis)) {
	    return false;
	}
    }
    return true;
}

bool WireCell::point_contained(const WireCell::Point& point, const WireCell::Ray& bounds, int axis)
{
    std::pair<double, double> mm = std::minmax(bounds.first[axis], bounds.second[axis]);
    return mm.first <= point[axis] && point[axis] <= mm.second;
}


double WireCell::point_angle(const WireCell::Vector& axis, const WireCell::Vector& vector)
{
    return acos(axis.dot(vector));
}

double WireCell::ray_length(const WireCell::Ray& ray)
{
    return (ray.second - ray.first).magnitude();
}

WireCell::Vector WireCell::ray_vector(const WireCell::Ray& ray)
{
    return ray.second - ray.first;
}
WireCell::Vector WireCell::ray_unit(const WireCell::Ray& ray)
{
    return ray_vector(ray).norm();
}

WireCell::Ray WireCell::ray_pitch(const WireCell::Ray& pu, const WireCell::Ray& qv)
{
    // http://geomalgorithms.com/a07-_distance.html
    const WireCell::Vector w0 = pu.first - qv.first;
    const WireCell::Vector u = ray_unit(pu);
    const WireCell::Vector v = ray_unit(qv);
    const double a = u.dot(u), b = u.dot(v), c = v.dot(v);
    const double d = u.dot(w0), e = v.dot(w0);

    const double denom = a*c - b*b;
    if (denom < 1e-6) {		// parallel
	double t = e/c;
	return Ray(pu.first, qv.first + t*v);
    }
    const double s = (b*e - c*d) / denom;
    const double t = (a*e - b*d) / denom;
    return Ray(pu.first + s*u, qv.first + t*v);
}

double WireCell::ray_dist(const WireCell::Ray& ray, const WireCell::Point& point)
{
    return ray_unit(ray).dot(point - ray.first);
}


WireCell::BoundingBox::BoundingBox(const Point& initial) : bounds(initial, initial) {}
WireCell::BoundingBox::BoundingBox(const Ray& initial) : bounds(initial) {}
void WireCell::BoundingBox::operator()(const Ray& r)
{
    (*this)(r.first);
    (*this)(r.second);
}
void WireCell::BoundingBox::operator()(const Point& p)
{
    for (int ind=0; ind<3; ++ind) {
	if (p[ind] < bounds.first[ind]) {
	    bounds.first[ind] = p[ind];
	}
	if (p[ind] > bounds.second[ind]) {
	    bounds.second[ind] = p[ind];
	}
    }
}
