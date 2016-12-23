#include "WireCellUtil/ImpactResponse.h"

using namespace WireCell;

ImpactResponse::ImpactResponse(const Response::Schema::PathResponse* pr)
    : m_pr(pr)
{
    m_spectrum = Waveform::dft(waveform());
}




PlaneImpactResponse::PlaneImpactResponse(const Response::Schema::FieldResponse& fr, int plane_number,
                                         double gain, double shaping)
    : m_fr(fr), m_plane_number(plane_number), m_half_extent(0.0), m_pitch(0.0), m_impact(0.0)
{
    auto pr = fr.planes[plane_number];

    //
    // FIXME HUGE ASSUMPTIONS ABOUT ORGANIZATION OF UNDERLYING
    // RESPONSE DATA!!!
    //
    // Paths must be in increasing pitch with one impact position at
    // nearest wire and 5 more impact positions equally spaced and at
    // smaller pitch distances than the associated wire.
    //

    const int n_per = 6;        // fixme: assumption
    const int n_wires = pr.paths.size()/n_per;
    const int n_wires_half = n_wires / 2; // integer div
    const int center_index = n_wires_half * n_per;

    m_impact = std::abs(pr.paths[1].pitchpos - pr.paths[0].pitchpos);
    m_half_extent = std::max(std::abs(pr.paths.front().pitchpos), std::abs(pr.paths.back().pitchpos));
    m_pitch = 2.0*std::abs(pr.paths[n_per-1].pitchpos - pr.paths[0].pitchpos);
    // std::cerr << "PIR:" << plane_number << ", impact:" << m_impact << ", pitch:" << m_pitch << ", half:" << m_half_extent << std::endl;

    // collect paths and index by wire and impact position.
    std::map<int, region_indices_t> wire_to_ind;
    for (int ind = 0; ind < pr.paths.size(); ++ind) {
        const Response::Schema::PathResponse* path = &pr.paths[ind];
        const int wirenum = int(ceil(path->pitchpos/pr.pitch)); // signed
        wire_to_ind[wirenum].push_back(ind);
        m_ir.push_back(new ImpactResponse(path));
        //std::cerr << "Wire:"<<wirenum<< "@" << ind << ": pitch=" << path->pitchpos << std::endl;
    }

    // apply symmetry.
    for (int irelwire=-n_wires_half; irelwire <= n_wires_half; ++irelwire) {
        auto direct = wire_to_ind[irelwire];
        auto other = wire_to_ind[-irelwire];

        std::vector<int> indices(direct.begin(), direct.end());
        for (auto it = other.rbegin()+1; it != other.rend(); ++it) {
            indices.push_back(*it);
        }
        m_bywire.push_back(indices);

        //std::cerr << irelwire << ":";
        for (auto num : indices) {
            //std::cerr << " [" << num << "]:" << m_ir[num]->path_response().pitchpos;
        }
        //std::cerr << std::endl;
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
    const int center_wire = nwires()/2;
    
    const int relwire = int(round(relpitch/m_pitch));
    const int wire_index = center_wire + relwire;
    
    const double remainder_pitch = relpitch - relwire*m_pitch;
    const int impact_index = int(round(remainder_pitch / m_impact)) + nimp_per_wire()/2;

    //std::cerr << "relpitch:" << relpitch << ", relwire:"<<relwire<<", wi:" << wire_index
    //          << ", rempitch=" << remainder_pitch << ", impactind=" << impact_index
    //          <<std::endl;
    return std::make_pair(wire_index, impact_index);
}

const ImpactResponse* PlaneImpactResponse::closest(double relpitch) const
{
    if (relpitch < -m_half_extent || relpitch > m_half_extent) {
        return nullptr;
    }
    std::pair<int,int> wi = closest_wire_impact(relpitch);
    if (wi.first < 0 || wi.first >= m_bywire.size()) {
        std::cerr << "PlaneImpactResponse::closest(): relative pitch: "
                  << relpitch
                  << " outside of wire range: " << wi.first
                  << std::endl;
        return nullptr;
    }
    auto region = m_bywire[wi.first];
    if (wi.second < 0 || wi.second >= region.size()) {
        std::cerr << "PlaneImpactResponse::closest(): relative pitch: "
                  << relpitch
                  << " outside of impact range: " << wi.second
                  << std::endl;
        return nullptr;
    }
    return m_ir[region[wi.second]];
}

PlaneImpactResponse::TwoImpactResponses PlaneImpactResponse::bounded(double relpitch) const
{
    if (relpitch < -m_half_extent || relpitch > m_half_extent) {
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


    const double absimpact = m_half_extent + relpitch - wi.first*m_pitch;
    const double sign = absimpact - wi.second*m_impact;

    if (sign > 0) {
        return std::make_pair(m_ir[region[wi.second]], m_ir[region[wi.second+1]]);
    }
    return std::make_pair(m_ir[region[wi.second-1]], m_ir[region[wi.second]]);
}




