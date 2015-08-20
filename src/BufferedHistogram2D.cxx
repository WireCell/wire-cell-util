#include "WireCellUtil/BufferedHistogram2D.h"

#include <iostream>
using namespace WireCell;
using namespace std;


BufferedHistogram2D::BufferedHistogram2D(double xbinsize, double ybinsize,
					 double xmin, double ymin)
    : m_xmin(xmin), m_ymin(ymin), m_xbinsize(xbinsize), m_ybinsize(ybinsize)
{
}


bool BufferedHistogram2D::fill(double x, double y, double v)
{
    int ybin = (y - m_ymin)/m_ybinsize;
    if (ybin < 0) {
	//cerr << "negative y bin="<<ybin<<" for y=" << y << endl;
	return false;
    }
    int xbin = (x - m_xmin)/m_xbinsize;
    if (xbin < 0) {
	//cerr << "negative x bin="<<xbin<<" for x=" << x << endl;
	return false;
    }

    m_xbindeque.resize(xbin+1);
    XBin& vec = m_xbindeque[xbin];
    vec.resize(ybin+1, 0.0);
    vec[ybin] += v;

    return true;
}

std::vector<double> BufferedHistogram2D::popx()
{
    std::vector<double> ret;

    if (!m_xbindeque.size()) { return ret; }

    ret = m_xbindeque.front();
    m_xbindeque.pop_front();
    m_xmin += m_xbinsize;
    return ret;
}
