/*
 * fftTests.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: saigopal nelaturi
 */

#include "ufabRV.h"




array indicator(array x){
	// returns the support of a scalar function x
	return (x > 0);
}

array sublevel(array x, double measure){
	// Actually this the complement of the sub-level sets defined in our papers
	// because we are computing convolution directly with the part, and not with
	// the part complement
	return (x < measure);
}

array maxRV(array x, array y, array y1){
	// In practice, the y has to be the INVERSE of the tool to calculate the
	// translational configuration space obstacle - could do conjugate if using
	// fft but sometimes direct convolution may be faster on the GPU, and
	// ArrayFire decides whether or not to use the FFT
	return dilate3(indicator(sublevel(convolve3(x,y,AF_CONV_EXPAND),1)),y1);
	//z = af::ifft3(af::fft3(x) * af::conjg(af::fft3(y)));

}


