// fixme: this file needs to move to util so gen can also use it

#ifndef WIRECELLSIGPROC_RESPONSE
#define WIRECELLSIGPROC_RESPONSE

#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Units.h"

#include "WireCellUtil/Point.h"

namespace WireCell {


    namespace Response {

	// These objects correspond to those defined in the Wire Cell
	// field response transfer file format schema.  
	namespace Schema {

	    /// Hold information about the induced current response
	    /// due to passage of a charge along one drift path.
	    struct PathResponse {
		WireCell::Waveform::realseq_t current;
		double pitchpos;
		double wirepos;
		PathResponse(const WireCell::Waveform::realseq_t& c, double p, double w)
		    : current(c), pitchpos(p), wirepos(w) {}

	    };


	    /// Hold information about the collection of induced
	    /// current responses on one wire plane.
	    struct PlaneResponse {
		std::vector<PathResponse> paths;
		int planeid;
		double pitch;
		WireCell::Vector pitchdir;
		WireCell::Vector wiredir;
		PlaneResponse(const std::vector<PathResponse>& paths, int pid, double p,
			      const WireCell::Vector& pdir, const WireCell::Vector& wdir)
		    : paths(paths), planeid(pid), pitch(p), pitchdir(pdir), wiredir(wdir) {} 

	    };
	    
	    /// Hold info about multiple plane responses in the detector.
	    struct FieldResponse {
		std::vector<PlaneResponse> planes;
		WireCell::Vector axis;
		double origin;
		double tstart;
		double period;
		FieldResponse(const std::vector<PlaneResponse>& planes, const WireCell::Vector& adir,
			      double o, double t, double p)
		    : planes(planes), axis(adir), origin(o), tstart(t), period(p) {}
	    };
	    
	    FieldResponse load(const char* filename);
	    void dump(const char* filename, const FieldResponse& fr);
	}


	/// Return a reduced FieldResponse structure where the
	/// Path::Response::current arrays are reduced by averaging
	/// over each wire region.
	Schema::FieldResponse wire_region_average(const Schema::FieldResponse& fr);

	/// Normalize all PathResponse::current arrays by the integral
	/// of the wire0 current of the last plane.  This modifies the
	/// FieldResponse structure in-place.
	void normalize_by_collection_integral(Schema::FieldResponse& fr);


	// some_matrix deconvolution(fr, ele, filter);


	/// The cold electronics response function.
	double coldelec(double time, double gain=7.8, double shaping=1.0);

	class Generator {
	public:
	    virtual ~Generator();
	    virtual double operator()(double time) const = 0;

	    /// Lay down the function into a binned waveform.
	    WireCell::Waveform::realseq_t generate(const WireCell::Waveform::Domain& domain, int nsamples);
	};

	/// A functional object caching gain and shape.
	class ColdElec : public Generator {
	    const double _g, _s;
	public:
	    // Create cold electronics response function.  Gain is an
	    // arbitrary scale, typically in mV/fC and shaping time in
	    // microsecond.  Shaping time in units consistent with
	    // calling the function.
	    ColdElec(double gain=7.8, double shaping=1.0);
	    virtual ~ColdElec();

	    // Return the response at given time.  Time in units consistent with shaping.
	    virtual double operator()(double time) const;

	};

	/// A functional object giving the response as a function of
	/// time to a simple RC circuit.
	class SimpleRC : public Generator {
	  const double _width, _offset, _tick;
	public:
	    // Create (current) response function for a simple RC
	    // circuit where a unit of charge is placed on the cap at
	    // time offset and circuit has RC time constant of given
	    // width.  Times are in units consistent with value used
	    // to call the function.
	    SimpleRC(double width, double tick=0.5*units::microsecond, double offset=0.0);
	    virtual ~SimpleRC();

	    // Return the response at a given time.  Time in units
	    // consistent with width and offset.  Warning: to get the
	    // delta function, one must call *exactly* at the offset
	    // time.
	    virtual double operator()(double time) const;

	};

    }
}

#endif
