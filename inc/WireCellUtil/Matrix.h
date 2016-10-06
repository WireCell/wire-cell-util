#ifndef WIRECELL_MATRIX
#define WIRECELL_MATRIX

#include "WireCellUtil/Waveform.h"

#include <Eigen/Core>

#include <memory>
#include <vector>


namespace WireCell {

    namespace Matrix {

	/// Try to hide dependency on Eigen and avoid "knowing"
	/// they point to Eigen3 matrices.
	typedef Eigen::MatrixXf real_matrix_imp;
	typedef Eigen::MatrixXcf complex_matrix_imp;


	/// All functions here take and give shared pointers.  It is
	/// these objects that user code should handle.
	typedef std::shared_ptr< real_matrix_imp > real_matrix;
	typedef std::shared_ptr< complex_matrix_imp > complex_matrix;

	/// Return a matrix built from a list of waveforms.
	real_matrix frame_matrix(const std::vector<Waveform::realseq_t>& waves);

	/// 2D discrete Fourier transform of real matrix.  It first
	/// does a by-row DFT and on that result a by-column DFT.  A
	/// new complex matrix is returned.
	complex_matrix dft(real_matrix mat);

	/// Perform 2D inverse, discrete Fourier transform.
	real_matrix idft(complex_matrix spec);

    }
}

#endif
