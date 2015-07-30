#include "WireCellUtil/BoundingBox.h"

WireCell::BoundingBox::BoundingBox(const Point& initial) : m_bounds(initial, initial) {}
WireCell::BoundingBox::BoundingBox(const Ray& initial) : m_bounds(initial) {}
void WireCell::BoundingBox::operator()(const Ray& r)
{
    (*this)(r.first);
    (*this)(r.second);
}
void WireCell::BoundingBox::operator()(const Point& p)
{
    for (int ind=0; ind<3; ++ind) {
	if (p[ind] < m_bounds.first[ind]) {
	    m_bounds.first[ind] = p[ind];
	}
	if (p[ind] > m_bounds.second[ind]) {
	    m_bounds.second[ind] = p[ind];
	}
    }
}

bool WireCell::BoundingBox::inside(const Point& point) const
{
    for (int ind=0; ind<3; ++ind) {
	if (point[ind] < m_bounds.first[ind]) return false;
	if (point[ind] > m_bounds.second[ind]) return false;
    }
    return true;
}
