#include "WireCellUtil/Persist.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Response.h"
#include <cmath>
#include <iostream>


using namespace WireCell;


Response::Schema::FieldResponse::~FieldResponse()
{
//    std::cerr << (void*)this << " FieldResponse "<<period<<" dying\n";
}

Response::Schema::PlaneResponse::~PlaneResponse()
{
//    std::cerr << (void*)this << " PlaneResponse "<<planeid<<" dying\n";
}

Response::Schema::PathResponse::~PathResponse()
{
//    std::cerr << (void*)this << " PathResponse "<<wirepos<<" dying\n";
}

/**
 frdict['FieldResponse'].keys()
 ['origin', 'axis', 'period', 'tstart', 'planes']

 frdict['FieldResponse']['planes'][0]['PlaneResponse'].keys()
 ['paths', 'planeid', 'pitchdir', 'wiredir', 'pitch']

 frdict['FieldResponse']['planes'][0]['PlaneResponse']['paths'][0]['PathResponse'].keys()
 ['current', 'wirepos', 'pitchpos']

 frdict['FieldResponse']['planes'][0]['PlaneResponse']['paths'][0]['PathResponse']['current']['array'].keys()
 ['shape', 'elements']

 */
WireCell::Response::Schema::FieldResponse WireCell::Response::Schema::load(const char* filename)
{
    //WireCell::ExecMon em("load");

    Json::Value top = WireCell::Persist::load(filename);
    Json::Value fr = top["FieldResponse"];

    using namespace WireCell::Response::Schema;

    std::vector<PlaneResponse> planes;
    
    //em("start conversion");
    for (auto plane : fr["planes"]) {
	//em("start plane");
	auto plr = plane["PlaneResponse"];

	std::vector<PathResponse> paths;
	for (auto path : plr["paths"]) {
	    auto par = path["PathResponse"];
	    WireCell::Waveform::realseq_t current;
	    for (auto c : par["current"]["array"]["elements"]) {
		current.push_back(c.asDouble());
	    }
	    auto wcpath = PathResponse(current, par["pitchpos"].asDouble(), par["wirepos"].asDouble());
	    paths.push_back(wcpath);
	    //std::cerr << "\tpath:" << wcpath.pitchpos << std::endl;
	}

	auto pdir = plr["pitchdir"];
	auto pitchdir = WireCell::Vector(pdir[0].asDouble(),pdir[1].asDouble(),pdir[2].asDouble());
	
	auto wdir = plr["wiredir"];
	auto wiredir = WireCell::Vector(wdir[0].asDouble(),wdir[1].asDouble(),wdir[2].asDouble());

	// std::cerr << "PLANE: " << plr["planeid"]
	// 	  << " pitchdir" << plr["pitchdir"]
	// 	  << " wiredir" << plr["wiredir"]
	// 	  << std::endl;


	//em("finish plane");
	PlaneResponse wcplr(paths, plr["planeid"].asInt(), plr["pitch"].asDouble(), pitchdir, wiredir);
	planes.push_back(wcplr);
	//em("make PlaneResponse");
	// std::cerr << "plane #" << wcplr.planeid
	// 	  << ": #paths=" << paths.size() << " from " << plr["paths"].size()
	// 	  << " pitchdir=" << wcplr.pitchdir << std::endl;
    }
    
    
    //em("done loop");

    auto adir = fr["axis"];
    auto axis = WireCell::Vector(adir[0].asDouble(),adir[1].asDouble(),adir[2].asDouble());
    auto ret = FieldResponse(planes, axis,
			     fr["origin"].asDouble(), fr["tstart"].asDouble(), fr["period"].asDouble());
    //em("returning");
    //std::cerr << em.summary() << std::endl;
    return ret;
}



void Response::Schema::dump(const char* filename, const Response::Schema::FieldResponse& fr)

{
}


void Response::Schema::lie(PlaneResponse& pr,
			   const WireCell::Vector& pitchdir,
			   const WireCell::Vector& wiredir)
{
    double dot = pitchdir.dot(wiredir);
    if (std::fabs(dot) > 0.00001) {
	std::cerr << "At least make your lies orthogonal." << std::endl;
	return;
    }

    pr.pitchdir = pitchdir;
    pr.wiredir = wiredir;
}


/// Warning!  this function is NOT GENERAL.  It is actually specific
/// to Garfield 1D line of paths with half the impact positions
/// represented!  
Response::Schema::FieldResponse Response::wire_region_average(const Response::Schema::FieldResponse& fr)
{
    using namespace WireCell::Waveform;
    using namespace WireCell::Response::Schema;

    std::vector<PlaneResponse> newplanes;
    for (auto plane : fr.planes) {
	std::vector<PathResponse> newpaths;

	double pitch = plane.pitch;

	std::map<int, realseq_t> avgs;
	std::map<int, int> nums;
	for (auto path : plane.paths) {
	    double adjusted_pitchpos = path.pitchpos;
	    if (adjusted_pitchpos > 0) {
		adjusted_pitchpos -= 0.001*pitch;
	    }
	    else {
		adjusted_pitchpos += -0.001*pitch;
	    }

	    const int region = round(adjusted_pitchpos/pitch);
	    const double impact = path.pitchpos - region*pitch;
	    const int nsamples = path.current.size();
	    if (avgs.find(region) == avgs.end()) {
		avgs[region] = realseq_t(nsamples);
	    }

	    // WARNING assumes units.
	    // WARNING assumes last impact is at 1.5mm.
	    // WARNING assumes impacts are on half-pitch lines of symmetry.
	    // WARNING assumes ~half the pitch not represented.
	    double weight = 2.0;
	    if (std::abs(impact) < 0.01 || std::abs(impact-1.5) < 0.01) { 
		weight = 1.0;	// don't double count central or last
	    }
	    realseq_t& response = avgs[region];
	    for (int ind=0; ind<nsamples; ++ind) {
		response[ind] += path.current[ind];
	    }
	    nums[region] += 1;
	}

	// do average.
	for (auto it : avgs) {
	    int region = it.first;
	    int num = nums[region];
	    realseq_t& response = it.second;
	    const int nsamples = response.size();
	    for (int ind=0; ind<nsamples; ++ind) {
		response[ind] /= num;
	    }

	    // pack up everything for return.
	    newpaths.push_back(PathResponse(response, region*pitch, 0.0));
	}
	newplanes.push_back(PlaneResponse(newpaths, plane.planeid, plane.pitch, plane.pitchdir, plane.wiredir));
    }
    return FieldResponse(newplanes, fr.axis, fr.origin, fr.tstart, fr.period);
}


void Response::normalize_by_collection_integral(Response::Schema::FieldResponse& fr)
{
}





Response::Generator::~Generator()
{
}

WireCell::Waveform::realseq_t Response::Generator::generate(const WireCell::Waveform::Domain& domain, int nsamples)
{
    WireCell::Waveform::realseq_t ret(nsamples);
    const double tick = (domain.second-domain.first)/nsamples;
    for (int ind=0; ind < nsamples; ++ind) {
	double t = domain.first + ind*tick;
	ret[ind] = (*this)(t);
    }
    return ret;
}





/*
  Cold Electronics response function.

  How was this function derived?

    1. Hucheng provided a transfer function of our electronics, which
       is the Laplace transformation of the shaping function.

    2. We did a anti-Laplace inverse of the shaping function

    3. The one you saw in the code is basically the result of the inversion

  - time_us is time in microsecond

  - gain_par is proportional to the gain, basically at 7.8 mV/fC, the
    peak of the shaping function should be at 7.8 mV/fC. In the code,
    you can find what value that I set to reach 14 mV/fC.

  - shaping_us is the shaping time (us)

  - the hard-coded numbers are the result of the inverting the
    Lapalace transformation in Mathematica.

 */
double Response::coldelec(double time, double gain, double shaping)
{
    if (time <=0 || time >= 10 * units::microsecond) { // range of validity
	return 0.0;
    }

    const double reltime = time/shaping;

    // a scaling is needed to make the anti-Lapalace peak match the expected gain
    // fixme: this scaling is slightly dependent on shaping time.  See response.py
    gain *= 10*1.012;

    return 4.31054*exp(-2.94809*reltime)*gain
	-2.6202*exp(-2.82833*reltime)*cos(1.19361*reltime)*gain
	-2.6202*exp(-2.82833*reltime)*cos(1.19361*reltime)*cos(2.38722*reltime)*gain
	+0.464924*exp(-2.40318*reltime)*cos(2.5928*reltime)*gain
	+0.464924*exp(-2.40318*reltime)*cos(2.5928*reltime)*cos(5.18561*reltime)*gain
	+0.762456*exp(-2.82833*reltime)*sin(1.19361*reltime)*gain
	-0.762456*exp(-2.82833*reltime)*cos(2.38722*reltime)*sin(1.19361*reltime)*gain
	+0.762456*exp(-2.82833*reltime)*cos(1.19361*reltime)*sin(2.38722*reltime)*gain
 	-2.620200*exp(-2.82833*reltime)*sin(1.19361*reltime)*sin(2.38722*reltime)*gain 
	-0.327684*exp(-2.40318*reltime)*sin(2.5928*reltime)*gain + 
	+0.327684*exp(-2.40318*reltime)*cos(5.18561*reltime)*sin(2.5928*reltime)*gain
	-0.327684*exp(-2.40318*reltime)*cos(2.5928*reltime)*sin(5.18561*reltime)*gain
	+0.464924*exp(-2.40318*reltime)*sin(2.5928*reltime)*sin(5.18561*reltime)*gain;
}

Response::ColdElec::ColdElec(double gain, double shaping)
    : _g(gain)
    , _s(shaping)
{
}
Response::ColdElec::~ColdElec()
{
}

double Response::ColdElec::operator()(double time) const
{
    return coldelec(time, _g, _s);
}


Response::SimpleRC::SimpleRC(double width, double tick, double offset)
  : _width(width), _tick(tick), _offset(offset)

{
}
Response::SimpleRC::~SimpleRC()
{
}
double Response::SimpleRC::operator()(double time) const
{
    double ret = -_tick/_width * exp(-(time-_offset)/_width);
    if (time == _offset) {	// this is a sketchy comparison
	ret += 1.0;		// delta function
    }
    return ret;
}


