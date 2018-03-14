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

void checkInputs(af::array part, af::array tool) {
	// check that the part and tool arrays are valid inputs

	assert(part.numdims() == tool.numdims()); // part and tool must be equi-dimensional
	int d = part.numdims(); // d-dimensional part
	assert(d == 2 || d == 3); // handling only 2 and 3-d.

	if (d == 2) {
		assert(part.dims()[0] == part.dims()[1]);
		assert(tool.dims()[0] == tool.dims()[1]);
	} else {
		assert(
				(part.dims()[0] == part.dims()[1])
						&& (part.dims()[0] == part.dims()[2]));
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

af::array computeEpsilonContactSpace(af::array part, af::array tool,
		angleAxis rotation, float epsilon) {
	/*
	 * Compute the contact space for an oriented tool. Doing this as
	 * a separate function to avoid memory issues.
	 */
	bool correlate = true; // need cross correlation
	return (sublevelComplement(
			convolveAF2(part,
					rotate(tool, rotation.angle, true,
							AF_INTERP_BICUBIC_SPLINE), correlate), epsilon));
}

int getBatchSize(int d, int partDim, int toolDim, int resultDim) {
	/*
	 * Estimate how many convolutions can fit on the gpu. The set
	 * of all convolutions that can fit on the gpu is the batch.
	 */
	//1. get allocatable memory (in bytes) available on the GPU
	double mem = getAvailableDeviceMemory();
	//2. estimate required memory for result and subtract from allocatable memory
	mem -= (pow(static_cast<double>(resultDim), static_cast<double>(d))
			* sizeof(f32));
	//3. estimate memory required for a single convolution
	// Each convolution needs to hold the part, tool and result on the GPU
	// TODO this is very inefficient because arrayfire stores the intermediate
	// convolutions on the GPU and does not go out of scope until the loop
	// exits. According to arrayfire this to avoid multiple reads and writes
	// which would result in overall performance degradation.
	double req = ceil(
			pow(static_cast<double>(partDim), static_cast<double>(d))
					* sizeof(f32));
	// add memory for the tool data separately in case part and tool input sizes differ
	req += ceil(
			pow(static_cast<double>(toolDim), static_cast<double>(d))
					* sizeof(f32));
	// add memory to store the convolution too (can't seem to delete this on the fly)
	req += ceil(
			pow(static_cast<double>(resultDim), static_cast<double>(d))
					* sizeof(f32));
	cout << "Available memory (MB) = " << mem / (1024 * 1024)
			<< " and memory required per batch (MB) = " << req / (1024 * 1024)
			<< endl;
	return floor(mem / req); // be conservative
}

af::array computeProjectedContactCSpace(af::array part, af::array tool,
		std::vector<angleAxis> rotations, float epsilon) {
	/*
	 * Given a part and a tool in d dimensions, compute the
	 * d* (d+1)/2 dimensional configuration space and extract
	 * the contact configurations, by identifying the configurations
	 * where the overlap measure is less than epsilon. Furthermore
	 * the support removal algorithm only requires the projection of
	 * this contact space.
	 */

	af::array projectedContactCSpace = constant(0,
			part.dims() + tool.dims() - 1, f32);

	int n = static_cast<int>(rotations.size()); //number of rotations
	af::timer::start();

	//gfor(seq i,n) {
	// see https://github.com/arrayfire/arrayfire/issues/1709
	for (int i = 0; i < n; i++) { // how to gfor this?
		// do cross correlation and return all voxels where the overlap field value is less than a measure;
		// TODO -- add fancy code to template whether to use convolveAF2 or AF3 depending on part.numdims()
		projectedContactCSpace += computeEpsilonContactSpace(part, tool,
				rotations[i], epsilon);
		af::eval(projectedContactCSpace); // this is required to avoid memory blowup, see github link above
		//printGPUMemory();
	}

	cout << "Done computing projected contact space in  " << af::timer::stop()
			<< " s" << endl;
	return projectedContactCSpace;

}

void removeSupports(af::array part, af::array tool,
		std::vector<angleAxis> rotations, float epsilon) {
	/*
	 * Recursive algorithm to remove supports
	 */
	af::array piContactCSpace = computeProjectedContactCSpace(part, tool,
			rotations, epsilon);

	if ((part.numdims() == 2)) {
		do {
			window.image(piContactCSpace);
		} while (!window.close());

	}

}
