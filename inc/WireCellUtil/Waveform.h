#ifndef WIRECELL_WAVEFORM
#define WIRECELL_WAVEFORM

#include <cstdint>

#include <Eigen/Core>		 // for Array
#include <unsupported/Eigen/FFT> // see type FFT below

namespace WireCell {

    namespace Waveform {

	// Type used for raw ADC values
	typedef uint16_t adc_t;

	// A sequence of ADC is what the FADC produces
	typedef Eigen::Array<adc_t, Eigen::Dynamic, 1> fadc_t;

	// Fundamental value type.
	typedef float value_t;
	// Complex used for example for discrete Fourier tranforms
	typedef std::complex<float> complex_t;

	// A real valued discrete waveforms.
	typedef Eigen::Array<value_t, Eigen::Dynamic, 1> signal_t;

	// Fourier transform of a signal
	typedef Eigen::Array<complex_t, Eigen::Dynamic, 1> fourier_t;


	// Discrete Fourier transform
	fourier_t fft(signal_t& wave);
	// Inverse, discrete Fourier transform
	signal_t ifft(fourier_t& spec);


	// Return the mean and (population) RMS over a waveform signal.
	std::pair<double,double> mean_rms(signal_t&sig);

    }
}

#endif
