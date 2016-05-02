#ifndef WIRECELL_WAVEFORM
#define WIRECELL_WAVEFORM

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

	/// Fundamental value type.
	typedef float value_t;

	/// Complex used for example for discrete Fourier tranforms
	typedef std::complex<float> complex_t;

	// A real valued discrete waveforms.
	typedef Eigen::Array<value_t, Eigen::Dynamic, 1> timeseq_t;

	/// Complex Fourier transform of a time sequence
	typedef Eigen::Array<complex_t, Eigen::Dynamic, 1> freqseq_t;

	/// Convert a std::vector into an Eigen::Array
	template<typename Val>			      // not const correct?
	Eigen::Array<Val, Eigen::Dynamic, 1> std2eig(/*const*/ std::vector<Val>& vec) {
	    typedef Eigen::Array<Val, Eigen::Dynamic, 1> cont_t;
	    return Eigen::Map<cont_t>(vec.data(), vec.size());
	}

	/// Convert an Eigen::Array into a std::vector
	template<typename Val>
	std::vector<Val> eig2std(/*const*/ Eigen::Array<Val, Eigen::Dynamic, 1>& vec) {
	    return std::vector<Val>(vec.data(), vec.data() + vec.size());
	}

	// Discrete Fourier transform
	freqseq_t fft(timeseq_t& wave);
	// Inverse, discrete Fourier transform
	timeseq_t ifft(freqseq_t& spec);


	// Return the mean and (population) RMS over a waveform signal.
	std::pair<double,double> mean_rms(timeseq_t& wave);

    }
}

#endif
