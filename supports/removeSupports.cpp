/*
 * removeSupports.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: nelaturi
 */


#include "removeSupports.h"
#include <assert.h>

void getRotations(int d){
	// sample SO(n)

}

af::array computeProjectedContactCSpace(af::array part, af::array tool, float epsilon){
	/*
	 * Given a part and a tool in d dimensions, compute the
	 * d* (d+1)/2 dimensional configuration space and extract
	 * the contact configurations, by identifying the configurations
	 * where the overlap measure is less than epsilon. Furthermore
	 * the support removal algorithm only requires the projection of
	 * this contact space.
	 */

	int partDim = part.dims()[0]; // part image size
	int toolDim = tool.dims()[0]; // tool image size
	int resultDim = partDim + toolDim -1; // convolution result size


	af::array projectedContactCSpace= constant(0,part.dims()+ tool.dims()-1, f32);

	af::timer::start();
	//gfor(seq i,n){
	int n = 10;
    for (int i = 0; i < n ; i++){   // how to gfor this?
        // do cross correlation and return all voxels where the overlap field value is less than a measure;
        af::array result = (sublevelComplement(
        		convolveAF2(part, rotate(tool,float(i*360.0/n), true, AF_INTERP_BICUBIC_SPLINE), true)
				,epsilon));
        result.as(f32);
        projectedContactCSpace += result;
    }

	cout << "Done computing projected contact space in  " << af::timer::stop() << " s" << endl;
	return projectedContactCSpace;

}

void removeSupports(af::array part, af::array tool, float epsilon){
	/*
	 * Recursive algorithm to remove supports
	 */

	assert(part.numdims() == tool.numdims()); // part and tool must be equi-dimensional
	int d = part.numdims(); // d-dimensional part

	// Check inputs
	assert (d == 2 || d == 3); // handling only 2 and 3-d.
	if (d == 2){
		assert (part.dims()[0] == part.dims()[1]);
		assert (tool.dims()[0] == tool.dims()[1]);
	} else {
		assert ((part.dims()[0] == part.dims()[1]) && (part.dims()[0] == part.dims()[2]) ) ;
		assert ((tool.dims()[0] == tool.dims()[1]) && (tool.dims()[0] == tool.dims()[2]) ) ;
	}

	getRotations(d);
	computeProjectedContactCSpace(part, tool, epsilon);



}
