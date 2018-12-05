/*
 * cspaceMorph.cpp
 *
 *  Created on: March 2, 2018
 *      Author: saigopal nelaturi
 */

#include "cspaceMorph.h"

#include "assert.h"

array indicator(array x) {
	// returns the support of a scalar function x as an array of floats
	// needed to force inputs to be indicators for the purposes of
	// convolution
	return (x > 0).as(f64);
}


array complement(array x){
	// return the complement of the array
	return ((1-x).as(f64));

}
array sublevel(array x, double measure) {
	// as defined in the group morphology paper
	return ((x >= (measure-0.001)).as(f64) );
}

double volume(array x) {
	// return number of non-zero elements
	array c = count(x);
	return 0; // FIX THIS!!!!!!!!!
}

array sublevelComplement(array x, double measure) {
	// This the complement of the sub-level sets defined in our papers
	// because we are computing convolution directly with the part, and not with
	// the part complement
	return (((x <= measure) && (x >= 1)).as(f64));
}

array levelSet(array x, double measure) {
	// This the complement of the sub-level sets defined in our papers
	// because we are computing convolution directly with the part, and not with
	// the part complement
	double tol = 1e-5;
	return ((x >= measure -tol) && (x <= measure + tol));
}

array reflect3(array x) {
	// compute the reflection of the shape, use Hermitian symmetry of DFT 
	return real(ifft3(conjg(fft3(x))));
}

array reflect2(array x) {
	// compute the reflection of the shape, use Hermitian symmetry of DFT
	return real(ifft2(conjg(fft2(x))));
}

array convolveAF3(array x, array y, bool correlate) {
	if (correlate) {
		return convolve3(x, reflect3(y), AF_CONV_DEFAULT, AF_CONV_AUTO);
	} else {
		return convolve3(x, y, AF_CONV_DEFAULT, AF_CONV_AUTO);
	}
}

array convolveAF2(array x, array y, bool correlate) {
	// Don't expand the convolution because it's not needed for the application
	if (correlate) {
		return convolve2(x, reflect2(y), AF_CONV_DEFAULT, AF_CONV_AUTO);
	} else {
		return convolve2(x, y, AF_CONV_DEFAULT, AF_CONV_AUTO);
	}
}

