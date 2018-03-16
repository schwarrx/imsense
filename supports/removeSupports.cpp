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

void checkInputs(af::array nearNet, af::array tool, af::array part) {
	// check that the nearNet and tool arrays are valid inputs

	assert(nearNet.numdims() == tool.numdims()); // nearNet and tool must be equi-dimensional
	assert(nearNet.numdims() == part.numdims()); // nearNet and part must be equi-dimensional
	int d = nearNet.numdims(); // d-dimensional nearNet
	assert(d == 2 || d == 3); // handling only 2 and 3-d.

	if (d == 2) {
		assert(nearNet.dims()[0] == nearNet.dims()[1]);
		assert(tool.dims()[0] == tool.dims()[1]);
		assert(part.dims()[0] == part.dims()[1]);
	} else {
		assert(
				(nearNet.dims()[0] == nearNet.dims()[1])
						&& (nearNet.dims()[0] == nearNet.dims()[2]));
		assert(
				(tool.dims()[0] == tool.dims()[1])
						&& (tool.dims()[0] == tool.dims()[2]));
		assert(
				(part.dims()[0] == part.dims()[1])
						&& (part.dims()[0] == part.dims()[2]));
	}
}

std::vector<angleAxis> getRotations(int d) {
	// sample SO(d) -- fill in this code
	std::vector<angleAxis> rotations;
	switch (d) {
	case 2: {
		cout << "sampling 2d rotations" << endl;
		// TODO this needs to be refined based on available gpu memory
		int n = 90; // evaluate 2d c-scpace at 4 degree increments
		for (int i = 0; i < n; i++) {
			angleAxis rot;
			rot.angle = double(i * 360 / n);
			rot.axis = Eigen::Vector3d(0, 0, 1); // assume rotation about z
			// axis is irrelevant for 2d rotations
			rotations.push_back(rot);
		}
		break;
	}
	case 3: {
		cout << "sampling 3d rotations" << endl;
		// not implemented yet -- use getRotationsFromFile in helper.h
		ifstream rotationFile;
		string filename = "576quaternions.dat";
		rotationFile.open(filename.c_str(), ios::in | ios::binary);
		if (!rotationFile) {
			cout << "Unable to open rotation file" << endl;
			exit(1); // terminate with error
		}
		string line;
		while (getline(rotationFile, line)) {
			std::istringstream iss(line);
			string w, x, y, z;
			iss >> w >> x >> y >> z;
			double qw = atof(w.c_str());
			double qx = atof(x.c_str());
			double qy = atof(y.c_str());
			double qz = atof(z.c_str());
			Eigen::Quaterniond q(qw, qx, qy, qz);
			// convert the quaternion q to angle axis
			Eigen::AngleAxisd aa(q);
			angleAxis rot;
			rot.angle = aa.angle();
			rot.axis = aa.axis();
			rotations.push_back(rot);
		}
		break;
	}
	}
	return (rotations);
}

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
	af::array components = af::regions(supports.as(b8), AF_CONNECTIVITY_4);
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

	af::array projectedContactCSpace = constant(0,
			nearNet.dims(), f32);

	int n = static_cast<int>(rotations.size()); //number of rotations
	af::timer::start();

	// see https://github.com/arrayfire/arrayfire/issues/1709
	for (int i = 0; i < n; i++) { // can we gfor this?
		// do cross correlation and return all voxels where the overlap field value is less than a measure;
		// TODO -- add fancy code to template whether to use convolveAF2 or AF3 depending on nearNet.numdims()
		projectedContactCSpace += getEpsilonContactSpace(nearNet, tool,
				rotations[i], epsilon);
		af::eval(projectedContactCSpace); // this is required to avoid memory blowup, see github link above
		// TODO -- do above in batches
		//printGPUMemory();
		// periodically do garbage collection .. lame. AF has some bug with non-expanded convolution
		if(n%10 ==0) af::deviceGC();
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
	af::array fracturePointLocations = af::where(
			piContactCSpace(af::where(dislocations)));

	// now identify which of the supports can be removed
	//af_print(components(af::where(fracturePointLocations(dislocations))));

	//af_print(trimmedPiContactCSpace(dislocations));
	//af_print(components(dislocations));
	if ((nearNet.numdims() == 2)) {
		do {
			window.image(piContactCSpace * dislocations);
		} while (!window.close());

	}
}

void runSupportRemoval(af::array nearNet, af::array tool, af::array part,
		float epsilon) {
	/*
	 * Run the support removal algorithm from start to end,
	 * including input checking and pre-processing
	 */
	checkInputs(nearNet, tool, part);


	int problemDimension = nearNet.numdims();
	std::vector<angleAxis> sampledRotations = getRotations(problemDimension);
	std::vector<int> maximallyRemovableSupports; // the output

	af::array supports = (nearNet - part); // the collection of all support structures
	float dilationKernelSize = 5;
	af::array dilatedPart = getDilatedPart(part, dilationKernelSize);
	af::array dislocations = getDislocationFeatures(dilatedPart, supports); // where the supports intersect the part
	af::array components = getSupportComponents(supports); // labeling all the supports by connected components
	//af::array componentDislocations = components(af::where(dislocations));

	// run the recursive support removal algo
	removeSupports(nearNet, tool, part, components, dislocations,
			sampledRotations, epsilon, maximallyRemovableSupports);

}
