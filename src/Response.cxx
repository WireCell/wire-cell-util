#include "WireCellUtil/Persist.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Response.h"
#include <cmath>
#include <iostream>
#include <set>


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
	//std::map<int, int> nums;

	
	std::map<int,realseq_t> fresp_map;
	std::map<int,std::pair<double,double>> pitch_pos_range_map;
	 
	// figure out the range of each response ... 

	int nsamples=0;
	
	for (auto path : plane.paths) {
	  int eff_num = path.pitchpos/(0.01 * pitch);
	  if (fresp_map.find(eff_num) == fresp_map.end()){
	    fresp_map[eff_num] = path.current;
	  }else{
	    nsamples = path.current.size();
	    for (int k=0;k!=nsamples;k++){
	      fresp_map[eff_num].at(k) = (fresp_map[eff_num].at(k) + path.current.at(k))/2.;
	    }
	  }
	  if (fresp_map.find(-eff_num) == fresp_map.end()){
	    fresp_map[-eff_num] = path.current;
	  }else{
	    int nsamples = path.current.size();
	    for (int k=0;k!=nsamples;k++){
	      fresp_map[-eff_num].at(k) = (fresp_map[-eff_num].at(k) + path.current.at(k))/2.;
	    }
	  }
	}


	std::vector<int> pitch_pos;
	for (auto it = fresp_map.begin(); it!= fresp_map.end(); it++){
	  pitch_pos.push_back((*it).first);
	}
	
	double min = -1e9;
	double max = 1e9;
	for (size_t i=0;i!=pitch_pos.size();i++){
	  int eff_num = pitch_pos.at(i);
	  if (i==0){
	    pitch_pos_range_map[eff_num] = std::make_pair(min,(pitch_pos.at(i) + pitch_pos.at(i+1))/2.*0.01*pitch);
	  }else if (i==pitch_pos.size()-1){
	    pitch_pos_range_map[eff_num] = std::make_pair((pitch_pos.at(i) + pitch_pos.at(i-1))/2.*0.01*pitch,max);
	  }else{
	    pitch_pos_range_map[eff_num] = std::make_pair((pitch_pos.at(i) + pitch_pos.at(i-1))/2.*0.01*pitch,(pitch_pos.at(i) + pitch_pos.at(i+1))/2.*0.01*pitch);
	  }
	}

	// for (size_t i=0;i!=pitch_pos.size();i++){
	//   int eff_num = pitch_pos.at(i)/(0.01*pitch);
	//   std::cout << i << " " << pitch_pos.at(i) << " " << eff_num << " " << pitch_pos_range_map[eff_num].first << " " << pitch_pos_range_map[eff_num].second << std::endl;
	// }

	// figure out how many wires ...
	std::set<int> wire_regions;
	for (size_t i=0;i!=pitch_pos.size();i++){
	  if (pitch_pos.at(i)>0){
	    wire_regions.insert( round((pitch_pos.at(i)*0.01*pitch-0.001*pitch)/pitch));
	  }else{
	    wire_regions.insert( round((pitch_pos.at(i)*0.01*pitch+0.001*pitch)/pitch));
	  }
	}
	

	// do the average ... 
	for(auto it = wire_regions.begin(); it!=wire_regions.end(); it++){
	  int wire_no = *it;
	  if (avgs.find(wire_no) == avgs.end()) {
	    avgs[wire_no] = realseq_t(nsamples);
	  }
	  for (auto it1 =  fresp_map.begin(); it1!= fresp_map.end(); it1++){
	    int resp_num = (*it1).first;
	    realseq_t& response = (*it1).second;
	    double low_limit = pitch_pos_range_map[resp_num].first;
	    double high_limit = pitch_pos_range_map[resp_num].second;
	    if (low_limit < (wire_no - 0.5)*pitch ){
	      low_limit = (wire_no - 0.5)*pitch;
	    }
	    if (high_limit > (wire_no+0.5)*pitch ){
	      high_limit = (wire_no+0.5)*pitch;
	    }

	    //
	    
	    if (high_limit > low_limit){
	      //std::cout << wire_no << " " << resp_num << " " << low_limit/pitch << " " << high_limit/pitch << std::endl;
	      for (int k=0;k!=nsamples;k++){
		avgs[wire_no].at(k) += response.at(k) * (high_limit - low_limit) / pitch;
	      }
	    }
	  }
	}
	

	
		

	  // realseq_t& response = avgs[region];
	  // for (int ind=0; ind<nsamples; ++ind) {
	  //   response[ind] += path.current[ind];
	  // }
	 
	
	// do average.
	for (auto it : avgs) {
	  int region = it.first;
	  realseq_t& response = it.second;

	  double sum = 0;
	  for (int k=0;k!=nsamples;k++){
	    sum += response.at(k);
	  }
	  //std::cout << plane.planeid << " " << sum*0.1*units::microsecond << std::endl;
	  
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



Array::array_xxf Response::as_array(const Schema::PlaneResponse& pr)
{
    int nrows = pr.paths.size();
    int ncols = pr.paths[0].current.size();
    Array::array_xxf ret(nrows, ncols); // warning, uninitialized

    for (int irow = 0; irow < nrows; ++irow) {
        auto& path = pr.paths[irow];
        for (int icol = 0; icol < ncols; ++icol) {
            ret(irow,icol) = path.current[icol]; // maybe there is a fast way to do this copy?
        }
    }
    return ret;        
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

double Response::hf_filter(double freq, double sigma, double power, bool flag){
  if (flag){
    if (freq==0) return 0;
  }

  return exp(-0.5*pow(freq/sigma,power));
  
}

double Response::lf_filter(double freq, double tau){
  return 1-exp(-pow(freq/tau,2));
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
    double ret = 0;
    if (_width > 0){
        ret += -_tick/_width * exp(-(time-_offset)/_width); // _tick here is to make this RC response integrated in each bin
    }
    if (time < _offset + _tick) {	// just the first bin 
       ret += 1.0;		// delta function
        //std::cerr<<"delta"<<std::endl;
    }
    return ret;
}


Response::LfFilter::LfFilter(double tau)
  : _tau(tau)
{
}

Response::LfFilter::~LfFilter(){
}

double Response::LfFilter::operator()(double freq) const
{
  return lf_filter(freq,_tau);
}


Response::HfFilter::HfFilter(double sigma, double power, bool flag)
  : _sigma(sigma)
  , _power(power)
  , _flag(flag)
{
}

Response::HfFilter::~HfFilter(){
}

double Response::HfFilter::operator()(double freq) const
{
  return hf_filter(freq,_sigma,_power,_flag);
}


