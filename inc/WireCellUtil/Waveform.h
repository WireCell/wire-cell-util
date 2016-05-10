#ifndef WIRECELL_WAVEFORM
#define WIRECELL_WAVEFORM

#include <cstdint>
#include <vector>
#include <complex>
#include <numeric>
#include <algorithm>

/*
 * Note, we do not expose any Eigen3 classes.
 */

namespace WireCell {


    namespace Waveform {

	/// The type for the signal in each bin.
	typedef float real_t;

	/// The type for the spectrum in each bin.
	typedef std::complex<float> complex_t;


	/// A sequence is an ordered array of values (real or
	/// complex).  By itself it is not associated with a domain.
	template<typename Val>
	using Sequence = std::vector<Val>;


	// A real-valued sequence, eg for discrete signal values.
	typedef Sequence<real_t> realseq_t;

	/// A complex-valued sequence, eg for discrete spectrum powers.
	typedef Sequence<complex_t> compseq_t;


	/// A half-open range of bins (from first bin to one past last bin)
	typedef std::pair<int,int> BinRange;

	/// A range of time
	typedef std::pair<double,double> Period;

	/// A range of frequency
	typedef std::pair<double,double> Band;


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

	/// Return the begin/end sample numbers inside the a subdomain of a domain with nsamples total.
	std::pair<int,int> sub_sample(const Domain& domain, int nsamples, const Domain& subdomain);
	    

	/// Return the real part of the sequence
	realseq_t real(const compseq_t& seq);
	/// Return the imaginary part of the sequence
	realseq_t imag(const compseq_t& seq);
	/// Return the magnitude or absolute value of the sequence
	realseq_t magnitude(const compseq_t& seq);
	/// Return the phase or arg part of the sequence
	realseq_t phase(const compseq_t& seq);


	/// Increase (shift) sequence values by scalar
	template<typename Val>
	void increase(Sequence<Val>& seq, Val scalar) {
	    std::transform(seq.begin(), seq.end(), seq.begin(), 
			   [scalar](Val x) { return x+scalar; });
	}
	inline void increase(Sequence<float>& seq, double scalar) {
	    increase(seq, (float)scalar);
	}

	/// Increase (shift) sequence values by values in another sequence
	template<typename Val>
	void increase(Sequence<Val>& seq, const Sequence<Val>& other) {
	    std::transform(seq.begin(), seq.end(), other.begin(), seq.begin(),
			   std::plus<Val>());
	}

	/// Scale (multiply) sequence values by scalar
	template<typename Val>
	void scale(Sequence<Val>& seq, Val scalar) {
	    std::transform(seq.begin(), seq.end(), seq.begin(), 
			   [scalar](Val x) { return x*scalar; });
	}
	inline void scale(Sequence<float>& seq, double scalar) {
	    scale(seq, (float)scalar);
	}

	/// Scale (multiply) seq values by values from the nother sequence.
	template<typename Val>
	void scale(Sequence<Val>& seq, const Sequence<Val>& other) {
	    std::transform(seq.begin(), seq.end(), other.begin(), seq.begin(),
			   std::multiplies<Val>());
	}
	/// Shrink (divide) seq values by values from the other sequence.
	template<typename Val>
	void shrink(Sequence<Val>& seq, const Sequence<Val>& other) {
	    std::transform(seq.begin(), seq.end(), other.begin(), seq.begin(),
			   std::divides<Val>());
	}




	/// Return sum of all entries in sequence.
	template<typename Val>
	Val sum(const Sequence<Val>& seq) {
	    return std::accumulate(seq.begin(), seq.end(), Val());
	}

	/// Return sum of square of all entries in sequence.
	template<typename Val>
	Val sum2(const Sequence<Val>& seq) {
	    return std::accumulate(seq.begin(), seq.end(), Val(),
				   [](const Val& bucket, Val x) {
				       return bucket + x*x;
				   });
	}
	
	// Return the mean and (population) RMS over a waveform signal.
	std::pair<double,double> mean_rms(const realseq_t& wave);

	// Return the median value.
	real_t median(realseq_t wave);

	/// Discrete Fourier transform of real sequence.  Returns full spectrum.
	compseq_t dft(realseq_t seq);

	/// Inverse, discrete Fourier transform.  Expects full
	/// spectrum, but only uses first half.
	realseq_t idft(compseq_t spec);
    }
}

#endif
