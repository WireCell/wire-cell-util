#ifndef WIRECELLUTIL_POINT_H
#define WIRECELLUTIL_POINT_H

#include "WireCellUtil/D3Vector.h"

#include <set>
#include <memory>		// auto_ptr

namespace WireCell {

    /// A 3D Cartesian point in double precision.

    typedef D3Vector<double> Point;

    /// An alias for Point.
    typedef Point Vector;

    /// A line segment with a begin and end point.
    typedef std::pair<Point, Point> Ray;

    /// PointVector - a collection of Points
    typedef std::vector<Point> PointVector;

    /// PointValue - an association of a point and a value
    typedef std::pair<Point, float> PointValue;

    /// PointValueVector - a collection of point-value associations
    typedef std::vector<PointValue> PointValueVector;

    /// Return true if lhs<rhs w/in tolerance.
    struct ComparePoints {
	bool operator()(const Point& lhs, const Point& rhs);
    };
    typedef std::set<Point, ComparePoints> PointSet;

    /// PointF - a 3D Cartesian point in single precision for when
    /// memory is constrained and double precision is not required.
    typedef D3Vector<float> PointF;


    
    /** Return true if point is contained in a rectangular solid
     * described by the ray bounds running between diagonally opposed
     * corners.*/
    bool point_contained(const Point& point, const Ray& bounds);

    /** Return true if point is contained by the bounding box along
     * the given axis (x=0, y=1, z=2) of the bounding box. */
    bool point_contained(const Point& point, const Ray& bounds, int axis);

}
std::ostream& operator<<(std::ostream& os, const WireCell::Ray& ray);



#endif

