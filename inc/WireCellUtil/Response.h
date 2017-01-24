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

		/// An array holding the induced current for the path on the wire-of-interest.
		WireCell::Waveform::realseq_t current;

		/// The position in the pitch direction to the starting point of the path, in millimeters.
		double pitchpos;

		/// The position along the wire direction to the starting point of the path, in millimeters.
		double wirepos;

                PathResponse() : pitchpos(0.0), wirepos(-99999.0) {}
		PathResponse(const WireCell::Waveform::realseq_t& c, double p, double w)
		    : current(c), pitchpos(p), wirepos(w) {}

                ~PathResponse();
	    };


	    /// Hold information about the collection of induced
	    /// current responses on one wire plane.
	    struct PlaneResponse {

		/// List of PathResponse objects.
		std::vector<PathResponse> paths;

		/// A numerical identifier for the plane.
		int planeid;

		/// The wire pitch in millimeters.
		double pitch;

		/// A normalized 3-vector giving direction of the wire pitch.
		WireCell::Vector pitchdir;

		/// A normalized 3-vector giving direction of the wire run.
		WireCell::Vector wiredir;

                PlaneResponse() : planeid(-1), pitch(0.0) {}
		PlaneResponse(const std::vector<PathResponse>& paths, int pid, double p,
			      const WireCell::Vector& pdir, const WireCell::Vector& wdir)
		    : paths(paths), planeid(pid), pitch(p), pitchdir(pdir), wiredir(wdir) {} 

                ~PlaneResponse();
	    };
	    
	    /// Hold info about multiple plane responses in the detector.
	    struct FieldResponse {

		/// List of PlaneResponse objects.
		std::vector<PlaneResponse> planes;

		/// A normalized 3-vector giving direction of axis (anti)parallel to nominal drift direction.
		WireCell::Vector axis;

		/// Location in millimeters on the axis where drift paths begin.
		double origin;

		/// Time in microseconds at which drift paths begin.
		double tstart;

		/// The sampling period in microseconds.
		double period;

                FieldResponse() : origin(-999.0), tstart(-999.0), period(0.0) {}
		FieldResponse(const std::vector<PlaneResponse>& planes, const WireCell::Vector& adir,
			      double o, double t, double p)
		    : planes(planes), axis(adir), origin(o), tstart(t), period(p) {}
                ~FieldResponse();
	    };
	    
	    FieldResponse load(const char* filename);
	    void dump(const char* filename, const FieldResponse& fr);

	    /// Affect a lie about the data by setting the orientation
	    /// of the wires for the given plane.
	    void lie(PlaneResponse& fr,
		     const WireCell::Vector& pitchdir, const WireCell::Vector& wiredir);
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
