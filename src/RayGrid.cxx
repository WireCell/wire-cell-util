#include "WireCellUtil/RayGrid.h"

#include <iostream>             // debug

using namespace WireCell;


RayGrid::RayGrid(const ray_pair_vector_t& rays, int normal_axis)
    : m_nrccs(rays.size())
    , m_pitch_mag(m_nrccs, 0.0)
    , m_pitch_dir(m_nrccs)
    , m_center(m_nrccs)
    , m_zero_crossing(m_nrccs, m_nrccs)
    , m_ray_jump(m_nrccs, m_nrccs)
    , m_a(boost::extents[m_nrccs][m_nrccs][m_nrccs])
    , m_b(boost::extents[m_nrccs][m_nrccs][m_nrccs])
{

    // really we are working in 2D space, so project all vectors into the plane.
    auto project = [normal_axis](Vector v) { v[normal_axis]=0.0; return v; };

    // must go through 1, 2 and 3 combonations

    // First, find the per-rccs things
    for (rccs_index_t irccs=0; irccs<m_nrccs; ++irccs) {
        const auto& rpair = rays[irccs];
        const auto& r0 = rpair.first;
        const auto& r1 = rpair.second;

        // vector of closest approach between the two parallel rays
        const auto rpitch = ray_pitch(r0, r1);

        // relative pitch vector
        auto rpv = project(rpitch.second - rpitch.first);

        m_pitch_mag[irccs] = rpv.magnitude();
        m_pitch_dir[irccs] = rpv.norm();

        // center point of ray 0
        m_center[irccs] = 0.5*(project(r0.first + r0.second));
    }

    // Next find cross-rccs things
    for (rccs_index_t il=0; il<m_nrccs; ++il) {
        for (rccs_index_t im=0; im<m_nrccs; ++im) {

            // ray pairs for rccs l and m
            const auto& rpl = rays[il];
            const auto& rpm = rays[im];

            const auto& rl0 = rpl.first;
            const auto& rl1 = rpl.second;
            const auto& rm0 = rpm.first;
            const auto& rm1 = rpm.second;

            // Iterate only over triangle of indices to avoid extra work.
            if (il < im) { 
                const auto r00 = ray_pitch(rl0, rm0);
                const auto& pl0 = r00.first;
                const auto& pm0 = r00.second;

                // These really should be the same after projection.
                m_zero_crossing(il,im) = project(pl0); 
                m_zero_crossing(im,il) = project(pm0);

                // along l-rccs ray 0, crossing of m-rccs ray 1.
                {
                    const auto ray = ray_pitch(rl0, rm1);
                    const auto jump = project(ray.first - pl0);
                    m_ray_jump(il, im) = jump;
                    // std::cerr << "il="<<il << " im="<<im
                    //           << " p1=" << ray.first << " p0=" << pl0
                    //           << " jump="<< jump
                    //           << " m_ray_array=" << m_ray_jump(il, im)
                    //           << std::endl;
                }
                // along m-rccs ray 0, crossing of l-rccs ray 1.
                {
                    const auto ray = ray_pitch(rm0, rl1);
                    const auto jump = project(ray.first - pm0);
                    m_ray_jump(im, il) = jump;
                }

            }
            if (il == im) {
                m_zero_crossing(il,im).invalidate();
                m_ray_jump(il,im).invalidate();
            }
        }
    }

    // Finally, find triple-rccs things (coefficients for
    // P^{lmn}_{ij}).  Needs some of the above completed.
    for (rccs_index_t in=0; in<m_nrccs; ++in) {
        const auto& pn = m_pitch_dir[in];
        const double cp = m_center[in].dot(pn);

        for (rccs_index_t il=0; il<m_nrccs; ++il) {
            if (il == in) { continue; }
            
            // triangle iteration
            for (rccs_index_t im=0; im<il; ++im) { 
                if (im == in) { continue; }

                const double rlmpn = m_zero_crossing(il,im).dot(pn);

                const double wlmpn = m_ray_jump(il,im).dot(pn);
                const double wmlpn = m_ray_jump(im,il).dot(pn);

                m_a[il][im][in] = wlmpn;
                m_a[im][il][in] = wmlpn;
                // b is symmetric.
                m_b[il][im][in] = rlmpn - cp;
                m_b[im][il][in] = rlmpn - cp;

                // j*a{lmn} + i*a{mln} + b{lmn}
            }
        }
    }
}

Vector RayGrid::zero_crossing(rccs_index_t one, rccs_index_t two) const
{
    return m_zero_crossing(one, two);
}

Vector RayGrid::ray_crossing(const ray_address_t& one, const ray_address_t& two) const
{
    const rccs_index_t l = one.rccs, m = two.rccs;
    const auto r00 = m_zero_crossing(l,m);
    const auto& wlm = m_ray_jump(l,m);
    const auto& wml = m_ray_jump(m,l);
    const double i = one.grid, j = two.grid;
    const auto res = r00 + j*wlm + i*wml;
    // std::cerr << "l="<<l << " m="<<m
    //           << " i="<<i<< " j=" << j
    //           << " wlm=" << wlm
    //           << " wml=" << wml
    //           << " res=" << res
    //           << std::endl;
    return res;
}

double RayGrid::pitch_location(const ray_address_t& one, const ray_address_t& two, rccs_index_t other) const
{
    const tensor_t::index il=one.rccs, im=two.rccs, in=other;
    const tensor_t::index i=one.grid, j=two.grid;
    return  j*m_a[il][im][in] + i*m_a[im][il][in] + m_b[il][im][in];
}
