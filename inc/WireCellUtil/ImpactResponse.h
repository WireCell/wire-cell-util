#ifndef WIRECELLGEN_IMPACTRESPONSE
#define WIRECELLGEN_IMPACTRESPONSE

#include "WireCellUtil/Response.h"
#include "WireCellUtil/Units.h"

namespace WireCell {

    /** The information about detector response at a particular impact.
     */
    class ImpactResponse {
	Waveform::compseq_t m_spectrum; // this is the only "real" data held
	const Response::Schema::PathResponse* m_pr;

    public:
	ImpactResponse(const Response::Schema::PathResponse* pr);

	/// Time-domain waveform of response 
	const Waveform::realseq_t& waveform() const { return m_pr->current; }

	/// Frequency-domain spectrum of response
	const Waveform::compseq_t& spectrum() const { return m_spectrum; }

	/// Corresponding path response
	const struct Response::Schema::PathResponse& path_response() const { return *m_pr; }
    };

    /** Collection of all impact responses for a plane */
    class PlaneImpactResponse {
    public:

	PlaneImpactResponse(const Response::Schema::FieldResponse& fr, int plane_number,
			    double gain=0.0, double shaping=0.0*units::us);
	~PlaneImpactResponse();
	    
	std::pair<int,int> closest_wire_impact(double relpitch) const;

	/// Return the response at the impact position closest to
	/// the given relative pitch location (measured relative
	/// to the wire of interest).
	const ImpactResponse* closest(double relpitch) const;

	/// Return the two responses which are associated with the
	/// impact positions on either side of the given pitch
	/// location (measured relative to the wire of interest).
	typedef std::pair<const ImpactResponse*,const ImpactResponse*> TwoImpactResponses;
	TwoImpactResponses bounded(double relpitch) const;

	typedef std::deque<int> region_indices_t;
	typedef std::vector<region_indices_t> wire_region_indicies_t;

	const wire_region_indicies_t& bywire_map() const { return m_bywire; }

	const Response::Schema::FieldResponse& field_response() const { return m_fr; }
	const Response::Schema::PlaneResponse& plane_response() const { return m_fr.planes[m_plane_number]; }

	double half_pitch() const { return m_half; }
	double pitch() const { return m_pitch; }
	double impact() const { return m_impact; }

	int nwires() const { return m_bywire.size(); }
	int nimp_per_wire() const { return m_bywire[0].size(); }

    private:
	const Response::Schema::FieldResponse& m_fr;
	int m_plane_number;

	wire_region_indicies_t m_bywire;

	std::vector<ImpactResponse*> m_ir;
	double m_half, m_pitch, m_impact;
    };

}

#endif

