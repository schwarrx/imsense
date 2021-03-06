/*
 * fftTests.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: saigopal nelaturi
 */

#include "ufabRV.h"

#include "assert.h"



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

array sublevelComplement(array x, double measure){
	// This the complement of the sub-level sets defined in our papers
	// because we are computing convolution directly with the part, and not with
	// the part complement
	return (x < measure);
}


array toolPlungeVolume(int length, int width, int depth){

	// Describe the volume removed at a single location as a function of the tool plunge depth
	// and the width of cut. We will assume the infinitesimal volume cut by a tool is a parallelopiped
	// whose dimensions are length * width * depth. This is consistent with how material removal rates
	// are calculated for milling. For now, we will assume the three dimensions are specified as 
	// number of voxels.TODO- rewrite this function
	// so it accepts units of length and convert that into voxels

	dim4 dim(length,width,depth);
	return constant(1, dim);

}

array reflect(array x){
	// compute the reflection of the shape, use Hermitian symmetry of DFT 
	return real(ifft3(conjg(fft3(x))));
}

array convolveAF(array x, array y, bool correlate){
    if(correlate){
        return convolve3(x,reflect(y),AF_CONV_EXPAND ,AF_CONV_AUTO);
    }
    else {
        return convolve3(x,y,AF_CONV_EXPAND , AF_CONV_AUTO);
    }
}

array maxRV (array x, array y, array infPocket) {

	 // Here x represents the Part, y represents the Tool Assembly, and infPocket represents the toolPlungeVolume at a location
	//return dilate3(indicator(sublevelComplement(convolve3(x,reflect(y),AF_CONV_EXPAND,AF_CONV_AUTO),1)),infPocket);
    return indicator(sublevelComplement(convolve3(x,reflect(y),AF_CONV_DEFAULT,AF_CONV_AUTO),1));
}





