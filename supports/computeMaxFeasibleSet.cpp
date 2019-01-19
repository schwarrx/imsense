/*
 * computeMaxFeasibleSet.cpp
 *
 *  Created on: Nov 29, 2018
 *      Author: nelaturi
 */

#include "computeMaxFeasibleSet.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <iterator>
#include <iomanip>      // std::setw
#include "helper.h"



af::array getMaxFeasibleSetPerOrientation(af::array obstacles, af::array tool,
		angleAxis rotation) {
	/*
	 * get the contact space for an oriented tool. Doing this as
	 * a separate function to avoid memory issues.
	 */

	bool correlate = true; // need cross correlation
	float level = 0.0;
	af::array cfree =  (levelSet(
			convolveAF2(obstacles,
					rotate(tool, rotation.angle, true,
							AF_INTERP_BICUBIC_SPLINE), correlate), level));
	return (cfree);
}

af::array maxFeasibleSet(af::array obstacles, af::array tool, af::array envelope) {

	af::deviceGC();

	// compute the maximal set where each point can be accessed by the tool
	// (in at least one orientation) without colliding with obstacles.
	obstacles = indicator(obstacles);
	tool = indicator(tool);

	assert(obstacles.numdims() == tool.numdims()); //inputs must be equi-dimensional

	int problemDimension = obstacles.numdims();
	std::vector<angleAxis> rotations = getRotations(problemDimension);

	af::array maxFeasible = constant(0, obstacles.dims(), f32);
	int n = static_cast<int>(rotations.size()); //number of rotations

	dim4 resultDim = obstacles.dims(); // convolution will not expand input size
	int batchsize = getBatchSize(problemDimension, obstacles.dims()[0],
			tool.dims()[0], resultDim[0]);

	cout <<"Batch size = " << batchsize <<  endl;
	// see https://github.com/arrayfire/arrayfire/issues/1709
	for (int i = 0; i < n; i++) { // can we gfor this?
		// do cross correlation and return all voxels where the overlap
		// field value is less than a measure;
		// TODO -- add fancy code to template whether to use
		// convolveAF2 or AF3 depending on nearNet.numdims()

		// IMPORTANT = ASSUME TOOL REFERENCE POINT IS AT IMAGE CENTER!!
		maxFeasible += getMaxFeasibleSetPerOrientation(obstacles, tool, rotations[i]);
		//visualize2D(maxFeasible+envelope);


		printGPUMemory();

		af::eval(maxFeasible);
		af::deviceGC();

		if (n % (batchsize/100) == 0) {
			// this is required to avoid memory blowup, see github link above
			//cout << "*************************** evaluating *********************************" << endl;
			af::eval(maxFeasible);
			// periodically do garbage collection .. lame.
			// AF has some bug with non-expanded convolution
			af::deviceGC();
		}
	}

	//af_print(maxFeasible);
	maxFeasible *= envelope; // intersect with the envelope (as a field)
//
	return maxFeasible;

}

void writeImages(af::array maxFeasible, af::array obstacles, af::array envelope, af::array envelopebd, af::array tool){
	// write images illustrating the approach (for papers)
	// first convert everything to floats
	maxFeasible = maxFeasible.as(f32);
	obstacles = obstacles.as(f32);
	envelope = envelope.as(f32);
	envelopebd = envelopebd.as(f32);

	// display the maxFeasible as a field in addition to the obstacles
	af::saveImage("maxFeasible_field.png", maxFeasible + 255*obstacles );

	// display the maxFeasible set
	maxFeasible = indicator(maxFeasible).as(f32);
	af::saveImage("maxFeasible_set.png", maxFeasible + 255* envelopebd + 255*obstacles);

	af::array conv = indicator(sublevel(convolveAF2(maxFeasible, tool, false),0)).as(f32);
	af::saveImage("tool_sweep.png", conv );


}
