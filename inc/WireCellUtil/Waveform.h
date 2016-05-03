#ifndef WIRECELL_WAVEFORM
#define WIRECELL_WAVEFORM

#include <iostream>		// for testing
#include <cstdint>
#include <vector>
#include <Eigen/Core>		 // for Array
#include <unsupported/Eigen/FFT> // see type FFT below

namespace WireCell {

    namespace Waveform {

	/// Type used for raw ADC values
	typedef uint16_t adc_t;

	/// A sequence of ADC is what the FADC produces
	typedef Eigen::Array<adc_t, Eigen::Dynamic, 1> fadc_t;

	/// Fundamental real value type.
	typedef float real_t;

	/// Complex used for example for discrete Fourier tranforms
	typedef std::complex<float> complex_t;

	/// A sequence is an ordered array of values (real or
	/// complex).  By itself it is not associated with a domain.
	template<typename Val>
	using Sequence = Eigen::Array<Val, Eigen::Dynamic, 1>;

	// A real-valued sequence.
	typedef Sequence<real_t> realseq_t;

	/// A complex-valued sequence.
	typedef Sequence<complex_t> compseq_t;

	/// A domain of a sequence is bounded by the time or frequency
	/// at the start of its first element and that at the end of
	/// its last.
	typedef std::pair<double,double> Domain;
	/// Return the number of samples needed to cover the domain with sample size width.
	inline int sample_count(const Domain& domain, double width) {
	    return (domain.second-domain.first)/width;
	}
	/// Return the sample size if domain is equally sampled with given number of samples
	inline double sample_width(const Domain& domain, int count) {
	    return (domain.second-domain.first)/count;
	}

	/// Convert a std::vector to a Sequence
	template<typename Val>			      // not const correct?
	Sequence<Val> std2eig(/*const*/ std::vector<Val>& vec) {
	    typedef Sequence<Val> cont_t;
	    return Eigen::Map<cont_t>(vec.data(), vec.size());
	}

	/// Convert a Sequence to a std::vector
	template<typename Val>
	std::vector<Val> eig2std(/*const*/ Sequence<Val>& vec) {
	    return std::vector<Val>(vec.data(), vec.data() + vec.size());
	}


	/// Return the real part of the sequence
	realseq_t real(const compseq_t& seq);
	/// Return the imaginary part of the sequence
	realseq_t imag(const compseq_t& seq);
	/// Return the magnitude or absolute value of the sequence
	realseq_t magnitude(const compseq_t& seq);
	/// Return the phase or arg part of the sequence
	realseq_t phase(const compseq_t& seq);

	/// Set a subdomain of a sequence with the given value.
	/// Samples that are modified will be fully greater than or
	/// equal the start of the subdomain and have no part greater
	/// than the end of the subdomain.
	template<typename Val>
	void mask(Sequence<Val>& seq, 
		  const Domain& domain, const Domain& subdomain,
		  const Val& value = Val())
	{
	    int siz = seq.size();
	    const double bin = sample_width(domain, siz);
	    const int beg = std::max(  0, int(     ceil((subdomain.first - domain.first) / bin)));
	    const int end = std::min(siz, int(siz- ceil((domain.second - subdomain.second) / bin)));
	    std::cerr << "Masking " << beg <<  " " << end << std::endl;
	    for (int ind=beg; ind<end; ++ind) {
		seq(ind) = value;
	    }
	}


	// Implementation: http://eigen.tuxfamily.org/index.php?title=EigenFFT

	/// Discrete Fourier transform of real sequence.  Returns full spectrum.
	compseq_t dft(realseq_t& seq);

	/// Inverse, discrete Fourier transform.  Expects full
	/// spectrum, but only uses first half.
	realseq_t idft(compseq_t& spec);


	// Return the mean and (population) RMS over a waveform signal.
	std::pair<double,double> mean_rms(realseq_t& wave);

	// Return the median value.
	real_t median(realseq_t& wave);

    }
}

#endif
