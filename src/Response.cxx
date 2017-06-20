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
    if (!filename) {
        std::cerr << "Response::Schema::load(): empty field response file name\n";
        return FieldResponse();
    }
    Json::Value top = WireCell::Persist::load(filename);
    if (top.isNull()) {
        std::cerr << "Response::Schema::load(): failed to load " << filename << "\n";
        return FieldResponse();
    }
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
	PlaneResponse wcplr(paths,
                            plr["planeid"].asInt(),
                            plr["location"].asDouble(),
                            plr["pitch"].asDouble(),
                            pitchdir, wiredir);
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
			     fr["origin"].asDouble(),
                             fr["tstart"].asDouble(),
                             fr["period"].asDouble(),
                             fr["speed"].asDouble());
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
	    //const double impact = path.pitchpos - region*pitch;
	    const int nsamples = path.current.size();
	    if (avgs.find(region) == avgs.end()) {
		avgs[region] = realseq_t(nsamples);
	    }

	    // WARNING assumes last impact is at 1/2 pitch.
	    // WARNING assumes impacts are on half-pitch lines of symmetry.
	    // WARNING assumes ~half the pitch not represented.
	    // double weight = 2.0;
	    // if (std::abs(impact) < 0.01*units::mm || std::abs(impact-1.5*units::mm) < 0.01*(0.5*pitch)) { 
	    //     weight = 1.0;	// don't double count central or last
	    // }
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
	newplanes.push_back(PlaneResponse(newpaths,
                                          plane.planeid,
                                          plane.location,
                                          plane.pitch,
                                          plane.pitchdir,
                                          plane.wiredir));
    }
    return FieldResponse(newplanes, fr.axis, fr.origin, fr.tstart, fr.period, fr.speed);
}


void Response::normalize_by_collection_integral(Response::Schema::FieldResponse& fr)
{
}





Response::Generator::~Generator()
{
}

// FIXME: eradicate Domain in favor of Binning
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
WireCell::Waveform::realseq_t Response::Generator::generate(const WireCell::Binning& tbins)
{
    const int nsamples = tbins.nbins();
    WireCell::Waveform::realseq_t ret(nsamples, 0.0);
    for (int ind=0; ind<nsamples; ++ind) {
        const double time = tbins.center(ind);
	ret[ind] = (*this)(time);
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

  - time is time in system of units

  - gain in units of volts/charge gives peak value of the response to
    a delta function of current integrating to unit charge.

  - shaping is the shaping time in system of units

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
  : _width(width), _offset(offset), _tick(tick)

{
    /* std::cerr<<"+++++++++++++++++++++++"<<std::endl; */
    /* std::cerr<<_width/units::ms<<" ms "<<_tick/units::us<<" us " */
    /*     <<_offset/units::us<<" us\n"; */
    /* std::cerr<<"unit? "<<_tick/_width<<std::endl; */
    /* std::cerr<<"+++++++++++++++++++++++"<<std::endl; */
}
Response::SimpleRC::~SimpleRC()
{
}
double Response::SimpleRC::operator()(double time) const
{
    double ret = -_tick/_width * exp(-(time-_offset)/_width); // _tick here is to make this RC response integrated in each bin
    if (time < _offset + _tick) {	// just the first bin 
	ret += 1.0;		// delta function
    std::cerr<<"delta"<<std::endl;
    }
    return ret;
}


