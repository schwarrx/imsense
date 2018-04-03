/*
 * removeSupports.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: nelaturi
 */

#include "removeSupports.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include "helper.h"
const static int width = 512, height = 512;
af::Window window(width, height, "2D plot example title");

af::array getDilatedPart(af::array part, float kernelSize) {
	// dilate the part -- useful for support intersections
	int d = part.numdims();
	af::array dilatedPart;

	if (d == 2) {
		af::array mask = constant(1, kernelSize, kernelSize);
		dilatedPart = dilate(part, mask);
	} else {
		af::array mask = constant(1, kernelSize, kernelSize, kernelSize);
		dilatedPart = dilate3(part, mask);
	}
	return (dilatedPart);
}

af::array getSupportComponents(af::array supports) {
	//get connected components -- need to cast input to b8 (binary 8 bit)
	// connected components will identify each support individually and label them.
	af::array components = af::regions(supports.as(b8), AF_CONNECTIVITY_8);
	// TODO -- add connected components code in 3d
	return (components);
}

af::array getDislocationFeatures(af::array dilatedPart, af::array supports) {
	/*
	 * Identify all the features of contact between each support and
	 * the part and store them. These will be needed when computing
	 * the contact fibers.
	 */
	// intersection is pointwise multiplication
	// identify where the dilated part intersects the supports
	af::array componentDislocations = dilatedPart * supports;
	return (componentDislocations);

}

af::array getEpsilonContactSpace(af::array nearNet, af::array tool,
		angleAxis rotation, float epsilon) {
	/*
	 * get the contact space for an oriented tool. Doing this as
	 * a separate function to avoid memory issues.
	 */
	bool correlate = true; // need cross correlation
	return (sublevelComplement(
			convolveAF2(nearNet,
					rotate(tool, rotation.angle, true,
							AF_INTERP_BICUBIC_SPLINE), correlate), epsilon));
}

af::array getProjectedContactCSpace(af::array nearNet, af::array tool,
		std::vector<angleAxis> rotations, float epsilon) {
	/*
	 * Given a nearNet and a tool in d dimensions, get the
	 * d* (d+1)/2 dimensional configuration space and extract
	 * the contact configurations, by identifying the configurations
	 * where the overlap measure is less than epsilon. Furthermore
	 * the support removal algorithm only requires the projection of
	 * this contact space.
	 */

	af::array projectedContactCSpace = constant(0, nearNet.dims(), f32);
	int n = static_cast<int>(rotations.size()); //number of rotations

	int d = tool.numdims(); // problem dimension
	dim4 resultDim = nearNet.dims(); // convolution will not expand input size
	int batchsize = getBatchSize(d, nearNet.dims()[0], tool.dims()[0],
			resultDim[0]);

	af::timer::start();

	// see https://github.com/arrayfire/arrayfire/issues/1709
	for (int i = 0; i < n; i++) { // can we gfor this?
		// do cross correlation and return all voxels where the overlap field value is less than a measure;
		// TODO -- add fancy code to template whether to use convolveAF2 or AF3 depending on nearNet.numdims()
		projectedContactCSpace += getEpsilonContactSpace(nearNet, tool,
				rotations[i], epsilon);
		//printGPUMemory();

		if (n % batchsize == 0) {
			af::eval(projectedContactCSpace); // this is required to avoid memory blowup, see github link above
			// periodically do garbage collection .. lame. AF has some bug with non-expanded convolution
			af::deviceGC();
		}

	}
	cout << "Done computing projected contact space in  " << af::timer::stop()
			<< " s" << endl;
	return (projectedContactCSpace);

}

void removeSupports(af::array nearNet, af::array tool, af::array part,
		af::array components, af::array dislocations,
		std::vector<angleAxis> rotations, float epsilon, std::vector<int> L) {
	/*
	 * Recursive algorithm to remove supports
	 * L is the 'list' (vector) of maximally removable supports (paper notation)
	 */

	// Compute the projected contact space
	af::array piContactCSpace = getProjectedContactCSpace(nearNet, tool,
			rotations, epsilon);

	// Now check if the trimmed projection contains some dislocation features.
	// To do this, check the value of the trimmed projection function at the
	// dislocation features. Then extract the locations where the trimmed
	// projection function is non-zero, i.e. where there is contact at a
	// dislocation feature. These are the contact points where the tool can fracture
	// the part with minimal interference.
	af::array accessibleDislocations = piContactCSpace * dislocations;
	af::array removableSupports =
			(components(af::where(accessibleDislocations)));
	af::array supportIndices = (setUnique(removableSupports));

	af_print(removableSupports);

	if ((nearNet.numdims() == 2)) {
		do {
			window.image(dislocations);
		} while (!window.close());

	}

	// now identify which of the supports can be removed
	//af_print(components(af::where(fracturePointLocations(dislocations))));

	//af_print(trimmedPiContactCSpace(dislocations));
	//af_print(components(dislocations));

}

void runSupportRemoval(af::array nearNet, af::array tool, af::array part,
		float epsilon) {
	/*
	 * Run the support removal algorithm from start to end,
	 * including input checking and pre-processing
	 */

	// force indicator functions for the inputs
	nearNet = indicator(nearNet);
	tool = indicator(tool);
	part = indicator(part);

	//af_print(nearNet(af::where(nearNet))); // check that indicators are correctly forced.

	checkInputs(nearNet, tool, part);

	int problemDimension = nearNet.numdims();
	std::vector<angleAxis> sampledRotations = getRotations(problemDimension);
	std::vector<int> maximallyRemovableSupports; // the output

	af::array supports = (nearNet - part); // the collection of all support structures

	float dilationKernelSize = 5; // how much to thicken the part to find the dislocation features
	af::array dislocations = getDislocationFeatures(
			getDilatedPart(part, dilationKernelSize), supports); // where the supports intersect the part

	af::array components = getSupportComponents(supports); // labeling all the supports by connected components

	// run the recursive support removal algo
	removeSupports(nearNet, tool, part, components, dislocations,
			sampledRotations, epsilon, maximallyRemovableSupports);

}
