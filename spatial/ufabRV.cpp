/*
 * fftTests.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: saigopal nelaturi
 */

#include "ufabRV.h"




array reflect_ft(array x){
	// reflect an array by taking fft conjugate - hermitian
	return real(ifft3(conjg(fft3(x.as(f32))))).as(x.type());
}

array indicator(array x){
	// returns the support of a scalar function x
	return (x > 0);
}

array sublevel(array x, double measure){
	return (x >= measure-0.0001);
}

double volume(array x){
	// return number of non-zero elements
	array c = count(x);
	//af_print(c);
	return 0;
}

array sublevel_complement(array x, double measure){
	// This the complement of the sub-level sets defined in our papers
	// because we are computing convolution directly with the part, and not with
	// the part complement
	return (x < measure);
}

array maxRVFT(array x, array y, array y1){
	array cspace = real(ifft3(fft3(x.as(f64) * conjg(fft3(y.as(f64))))));
	return cspace;

}

array maxRV(array x, array y, array y1){
	// In practice, the y has to be the INVERSE of the tool to calculate the
	// translational configuration space obstacle - could do conjugate if using
	// fft but sometimes direct convolution may be faster on the GPU, and
	// ArrayFire decides whether or not to use the FFT
	//return dilate3(indicator(sublevel_complement(convolve3(x,y,AF_CONV_EXPAND),1)),y1);
	return convolve3(x,y,AF_CONV_EXPAND);


}


