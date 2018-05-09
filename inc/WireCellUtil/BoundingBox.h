#ifndef WIRECELL_BOUNDINGBOX
#define WIRECELL_BOUNDINGBOX

#include "WireCellUtil/Point.h"


namespace WireCell {

    /** A bounding box parallel to the Cartesian axes.
     */
    class BoundingBox {
	Ray m_bounds;
    bool initialized = false;
        
    public:
	
	/// Return true if point is inside bounding box
	bool inside(const Point& point) const;

	/// Return the ray representing the bounds.
	const Ray& bounds() const { return m_bounds; }

    /// Create a bounding box without an initial point or ray
    BoundingBox(){ m_bounds = Ray();}
	
    /// Create a bounding box bounding an initial point.
	BoundingBox(const Point& initial);

	/// Create a bounding box bounding an initial ray.
	BoundingBox(const Ray& initial);

	/// Create a bounding box from an iterator pair.
	template<typename RayOrPointIterator>
	BoundingBox(const RayOrPointIterator& begin, const RayOrPointIterator& end) {
	    for (auto it = begin; it != end; ++it) { (*this)(*it); }
	}

	/// Enlarge bounds to hold point.
	void operator()(const Point& p);

	/// Enlarge bounds to hold ray.
	void operator()(const Ray& r);

	template<typename RayOrPointIterator>
	void operator()(const RayOrPointIterator& begin, const RayOrPointIterator& end) {
	    for (auto it = begin; it != end; ++it) { (*this)(*it); }
	}


    };

    
}

#endif
