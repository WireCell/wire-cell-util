#include "WireCellUtil/Matrix.h"

#include <unsupported/Eigen/FFT>

#include <algorithm>

using namespace WireCell;

Matrix::real_matrix Matrix::frame_matrix(const std::vector<Waveform::realseq_t>& waves)
{
    const int nwaves = waves.size();
    std::vector<int> nticksper;
    for (auto wave : waves) {
	nticksper.push_back(wave.size());
    }
    const int max_nticks = *std::max_element(nticksper.begin(), nticksper.end());
    Matrix::real_matrix ret = std::make_shared<real_matrix_imp>(nwaves, max_nticks);
    for (int iwave=0; iwave<nwaves; ++iwave) {
	const Waveform::realseq_t& wave = waves[iwave];
	const int nticks = wave.size();
	for (int itick=0; itick<nticks; ++itick) {
	    (*ret)(iwave, itick) = wave[itick];
	}
    }
    return ret;
}

//http://stackoverflow.com/a/33636445
Matrix::complex_matrix Matrix::dft(Matrix::real_matrix mat)
{
    const int nrows = mat->rows();
    const int ncols = mat->cols();

    Matrix::complex_matrix ret = std::make_shared<Matrix::complex_matrix_imp> (nrows, ncols);

    Eigen::FFT< float > fft;
    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXcf fspec(ncols); // frequency spectrum 
        fft.fwd(fspec, mat->row(irow));
        ret->row(irow) = fspec;
    }

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows); // periodicity spectrum
        fft.fwd(pspec, ret->col(icol));
        ret->col(icol) = pspec;
    }

    return ret;
}

Matrix::real_matrix Matrix::idft(Matrix::complex_matrix mat)
{
    const int nrows = mat->rows();
    const int ncols = mat->cols();

    Matrix::complex_matrix partial = std::make_shared<Matrix::complex_matrix_imp> (nrows, ncols);

    Eigen::FFT< float > fft;

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows); // periodicity spectrum
        fft.inv(pspec, mat->col(icol));
        partial->col(icol) = pspec;
    }

    Matrix::real_matrix ret = std::make_shared<Matrix::real_matrix_imp> (nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXf wave(ncols); // frequency spectrum 
        fft.inv(wave, partial->row(irow));
        ret->row(irow) = wave;
    }

    return ret;

}

// Waveform::compseq_t WireCell::Waveform::cdftfwd(compseq_t wave)
// {
//     auto v = Eigen::Map<Eigen::VectorXcf>(wave.data(), wave.size());
//     Eigen::FFT<Waveform::real_t> trans;
//     Eigen::VectorXcf ret = trans.fwd(v);
//     return compseq_t(ret.data(), ret.data()+ret.size());
// }

// Waveform::compseq_t WireCell::Waveform::cdftinv(compseq_t spec)
// {
//     Eigen::FFT<Waveform::real_t> trans;
//     auto v = Eigen::Map<Eigen::VectorXcf>(spec.data(), spec.size());
//     Eigen::VectorXcf ret;
//     trans.inv(ret, v);
//     return compseq_t(ret.data(), ret.data()+ret.size());
// }

