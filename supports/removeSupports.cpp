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

void checkInputs(af::array nearNet, af::array tool) {
	// check that the nearNet and tool arrays are valid inputs

	assert(nearNet.numdims() == tool.numdims()); // nearNet and tool must be equi-dimensional
	int d = nearNet.numdims(); // d-dimensional nearNet
	assert(d == 2 || d == 3); // handling only 2 and 3-d.

	if (d == 2) {
		assert(nearNet.dims()[0] == nearNet.dims()[1]);
		assert(tool.dims()[0] == tool.dims()[1]);
	} else {
		assert(
				(nearNet.dims()[0] == nearNet.dims()[1])
						&& (nearNet.dims()[0] == nearNet.dims()[2]));
		assert(
				(tool.dims()[0] == tool.dims()[1])
						&& (tool.dims()[0] == tool.dims()[2]));
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
	return rotations;
}

void computeDislocationFeatures(af::array nearNet, af::array part) {
	/*
	 * Identify all the features of contact between each support and
	 * the part and store them. These will be needed when computing
	 * the contact fibers.
	 */
	af::array supports = (nearNet - part);
	//compute connected components -- need to cast input to b8 (binary 8 bit)
	// connected components will identify each support individually and label them.
	af::array components = af::regions(supports.as(b8), AF_CONNECTIVITY_4);

	// now find the points of intersection
	// first dilate the part
	int d = part.numdims();
	af::array dilatedPart;

	if (d == 2) {
		af::array mask = constant(1, 3, 3);
		dilatedPart = dilate(part, mask);
	} else {
		af::array mask = constant(1, 2, 2, 2);
		dilatedPart = dilate3(part, mask);
	}
	// intersection is pointwise multiplication
	af::array dislocationidx = af::where(dilatedPart * supports);
	af::array labeledDislocations = components(dislocationidx);
	std::vector<std::pair<int,int> > dislocationMap;
	gfor(seq i,dislocationidx.dims()[0])
	{
		std::pair<int, int> val = std::make_pair(0,0); // change this~
		dislocationMap.push_back(val);

	}

}

af::array computeEpsilonContactSpace(af::array nearNet, af::array tool,
		angleAxis rotation, float epsilon) {
	/*
	 * Compute the contact space for an oriented tool. Doing this as
	 * a separate function to avoid memory issues.
	 */
	bool correlate = true; // need cross correlation
	return (sublevelComplement(
			convolveAF2(nearNet,
					rotate(tool, rotation.angle, true,
							AF_INTERP_BICUBIC_SPLINE), correlate), epsilon));
}

af::array computeProjectedContactCSpace(af::array nearNet, af::array tool,
		std::vector<angleAxis> rotations, float epsilon) {
	/*
	 * Given a nearNet and a tool in d dimensions, compute the
	 * d* (d+1)/2 dimensional configuration space and extract
	 * the contact configurations, by identifying the configurations
	 * where the overlap measure is less than epsilon. Furthermore
	 * the support removal algorithm only requires the projection of
	 * this contact space.
	 */

	af::array projectedContactCSpace = constant(0,
			nearNet.dims() + tool.dims() - 1, f32);

	int n = static_cast<int>(rotations.size()); //number of rotations
	//af::timer::start();

	// see https://github.com/arrayfire/arrayfire/issues/1709
	for (int i = 0; i < n; i++) { // can we gfor this?
		// do cross correlation and return all voxels where the overlap field value is less than a measure;
		// TODO -- add fancy code to template whether to use convolveAF2 or AF3 depending on nearNet.numdims()
		projectedContactCSpace += computeEpsilonContactSpace(nearNet, tool,
				rotations[i], epsilon);
		af::eval(projectedContactCSpace); // this is required to avoid memory blowup, see github link above
		//printGPUMemory();
	}
	/*cout << "Done computing projected contact space in  " << af::timer::stop()
	 << " s" << endl;*/
	return projectedContactCSpace;

}

void removeSupports(af::array nearNet, af::array tool,
		std::vector<angleAxis> rotations, float epsilon) {
	/*
	 * Recursive algorithm to remove supports
	 */
	af::array piContactCSpace = computeProjectedContactCSpace(nearNet, tool,
			rotations, epsilon);

	/*	if ((nearNet.numdims() == 2)) {
	 do {
	 window.image(piContactCSpace);
	 } while (!window.close());

	 }*/

}
