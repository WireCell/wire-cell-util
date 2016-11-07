/**
   Wire Cell uses Eigen3 arrays for holding large block data like the
   waveforms from one plane of one readout frame.  This header
   provides a shim between Eigen3 and the rest of Wire Cell.

   There are a few important rules:
 
   - Eigen3 Arrays have element-wise arithmetic and Matrices have
     matrix-like arithmetic otherwise are similar.

   - They have .array() and .matrix() methods to produce one from the other.

   - Arrays are indexed by (row,col) order.  

   - An Eigen3 Vector is a 1D Matrix of shape (N,1), again, (row,col).

   - A row, column or block from an array references the original
     array so can not live beyond it.  

   - In Wire Cell large arrays are accessed via const shared pointer.

   Usage examples are given below.
 */ 


#ifndef WIRECELL_ARRAY
#define WIRECELL_ARRAY

#include "WireCellUtil/Waveform.h"

#include <Eigen/Core>

#include <memory>
#include <vector>


namespace WireCell {

    namespace Array {

	// template <typename Derived>
	// using shared_dense = std::shared_ptr< Eigen::DenseBase<Derived> >;
	// template <typename Derived>
	// using const_shared_dense = std::shared_ptr< const Eigen::DenseBase<Derived> >;


	/** When creating a new array, do so like:

	    const_shared_array_xxf my_great_array(int nrows, int ncols) {
	        shared_array_xxf arr = std::make_shared<WireCell::Array::array_xxf>(nrows, ncols);
		// now fill array
		return arr;	// casts to const
	    }

	    // ...

	    const_shared_array_xxf mga = my_great_array(3000,10000);
	    // use mga...
	    // explicitly drop
	    mga = nullptr;
	    // or wait for it to leave scope
	 */
	// real array
	typedef Eigen::ArrayXXf array_xxf;
	typedef std::shared_ptr< array_xxf > shared_array_xxf;
	typedef std::shared_ptr< const array_xxf > const_shared_array_xxf;

	// complex array
	typedef Eigen::ArrayXXcf array_xxc;
	typedef std::shared_ptr< array_xxc > shared_array_xxc;
	typedef std::shared_ptr< const array_xxc > const_shared_array_xxc;

	

	/** Perform 2D discrete Fourier transform.

	    This first performs 1D DFTs on individual rows and then 1D
	    DFTs on resulting columns.

	    const_shared_array_xxf arr = ...;
	    const_shared_array_xxc spec = dft(*arr);

	    // ...

	    const_shared_array_xxf arr2 = idft(*spec);
	 */
	array_xxc dft(const array_xxf& arr);
	array_xxf idft(const array_xxc& arr);

	/** Perform 2D deconvolution. 
	    
	    This will perform a 2D forward DFT, do an
	    element-by-element multiplication of that
	    periodicity/frequency space matrix by the filter and then
	    perform an 2D inverse DFT.

	 */
	array_xxf deconv(const array_xxf& arr, const array_xxc& filter);


    }
}

#endif
