#ifndef WIRECELLUTIL_POINT_H
#define WIRECELLUTIL_POINT_H

#include "WireCellUtil/D3Vector.h"

namespace WireCell {

    /// Point - a 3D Cartesian point in double precision.
    typedef WireCell::D3Vector<double> Point;

    /// PointPair - a pair of double precision points.
    typedef std::pair< WireCell::Point, WireCell::Point > PointPair;

    /// PointVector - a collection of Points
    typedef std::vector<WireCell::Point> PointVector;

    /// PointValue - an association of a point and a value
    typedef std::pair<WireCell::Point, float> PointValue;

    /// PointValueVector - a collection of point-value associations
    typedef std::vector<WireCell::PointValue> PointValueVector;


    /// PointF - a 3D Cartesian point in single precision for when
    /// memory is constrained and double precision is not required.
    typedef WireCell::D3Vector<float> PointF;

}

#endif

