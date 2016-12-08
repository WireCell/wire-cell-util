#include "WireCellUtil/ImpactResponse.h"

using namespace WireCell;

ImpactResponse::ImpactResponse(const Response::Schema::PathResponse* pr)
    : m_pr(pr)
{
    //... calc spectrum
}




PlaneImpactResponse::PlaneImpactResponse(const Response::Schema::FieldResponse& fr, int plane_number,
					 double gain, double shaping)
    : m_half(0.0), m_pitch(0.0), m_impact(0.0)
{
    auto pr = fr.planes[plane_number];

    //
    // FIXME HUGE ASSUMPTIONS ABOUT ORGANIZATION OF UNDERLYING
    // RESPONSE DATA!!!
    //
    // Paths must be in increasing pitch with one impact position at
    // nearest wire and 5 more impact positions equally spaced and
    // "above" wire in the pitch direction.
    //

    const int n_per = 6;
    const int n_wires = pr.paths.size()/n_per;
    const int n_wires_half = n_wires / 2; // integer div
    const int center_index = n_wires_half * n_per;

    m_impact = pr.paths[1].pitchpos - pr.paths[0].pitchpos;
    m_half = pr.paths.back().pitchpos;
    m_pitch = 2.0*(pr.paths[n_per-1].pitchpos - pr.paths[0].pitchpos);

    for (int iwire=0; iwire < n_wires; ++iwire) {

	const int icenter_m = iwire*n_per;
	const int icenter_p = center_index + (n_wires_half - iwire)*n_per;
	
	region_indices_t region_indices;
	for (int iimp=0; iimp<n_per; ++iimp) {
	    region_indices.push_back(icenter_m + iimp);
	    if (iimp) {		// skip doubled center
		region_indices.push_front(icenter_p + iimp);
	    }
	}
	m_bywire.push_back(region_indices);
    }

    for (int ind=0; ind<pr.paths.size(); ++ind) {
	m_ir.push_back(new ImpactResponse(&pr.paths[ind]));
    }

}

PlaneImpactResponse::~PlaneImpactResponse()
{
    for (auto tokill : m_ir) {
	delete tokill;
	tokill = 0;
    }
    m_ir.clear();
}


std::pair<int,int> PlaneImpactResponse::closest_wire_impact(double relpitch) const
{
    const double abspitch = m_half + relpitch;
    const int wire_index = int(abspitch/m_pitch);

    const double absimpact = abspitch - wire_index*m_pitch;
    const int impact_index = int(round(absimpact/m_impact));

    return std::make_pair(wire_index, impact_index);
}

const ImpactResponse* PlaneImpactResponse::closest(double relpitch) const
{
    if (relpitch < -m_half || relpitch > m_half) {
	return 0;
    }
    std::pair<int,int> wi = closest_wire_impact(relpitch);
    auto region = m_bywire[wi.first];
    return m_ir[region[wi.second]];
}

PlaneImpactResponse::TwoImpactResponses PlaneImpactResponse::bounded(double relpitch) const
{
    if (relpitch < -m_half || relpitch > m_half) {
	return std::pair<const ImpactResponse*, const ImpactResponse*>(nullptr, nullptr);
    }

    std::pair<int,int> wi = closest_wire_impact(relpitch);

    auto region = m_bywire[wi.first];
    if (wi.second == 0) {
	return std::make_pair(m_ir[region[0]], m_ir[region[1]]);
    }
    if (wi.second == region.size()-1) {
	return std::make_pair(m_ir[region[wi.second-1]], m_ir[region[wi.second]]);
    }


    const double absimpact = m_half + relpitch - wi.first*m_pitch;
    const double sign = absimpact - wi.second*m_impact;

    if (sign > 0) {
	return std::make_pair(m_ir[region[wi.second]], m_ir[region[wi.second+1]]);
    }
    return std::make_pair(m_ir[region[wi.second-1]], m_ir[region[wi.second]]);
}




