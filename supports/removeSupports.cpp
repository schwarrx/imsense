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

angleAxis convertQtoAngleAxis(Eigen::Quaterniond q){
	// convert a quaternion to angle axis
	double angle = 2* acos(q.w());
	double x = q.x()/sqrt(1- q.w()*q.w());
	double y = q.y()/sqrt(1- q.w()*q.w());
	double z = q.z()/sqrt(1- q.w()*q.w());
	Eigen::Vector3d axis(x,y,z);
	angleAxis rotation;
	rotation.angle = angle;
	rotation.axis = axis;
	return rotation;
}

std::vector<angleAxis> getRotations(int d) {
	// sample SO(d) -- fill in this code
	std::vector<angleAxis> rotations;
	switch (d) {
	case 2: {
		cout << "sampling 2d rotations" << endl;
		// TODO this needs to be refined based on available gpu memory
		int n = 20; // 20 rotations for now on Sai's machine
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
			//cout << line << '\n';
			std::istringstream iss(line);
			string w, x, y, z;
			iss >> w >> x >> y >> z;
			double qw = atof(w.c_str());
			double qx = atof(x.c_str());
			double qy = atof(y.c_str());
			double qz = atof(z.c_str());
			Eigen::Quaterniond q(qw, qx, qy, qz);
			// convert the quaternion q to angle axis
			angleAxis rot =convertQtoAngleAxis(q);
			rotations.push_back(rot);
		}
		break;
	}
	}
	return rotations;
}

af::array computeProjectedContactCSpace(af::array part, af::array tool,
		float epsilon) {
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
	int resultDim = partDim + toolDim - 1; // convolution result size

	af::array projectedContactCSpace = constant(0,
			part.dims() + tool.dims() - 1, f32);

	af::timer::start();
	//gfor(seq i,n){
	int n = 20;
	for (int i = 0; i < n; i++) {   // how to gfor this?
		// do cross correlation and return all voxels where the overlap field value is less than a measure;
		// TODO -- add fancy code to template whether to use convolveAF2 or AF3 depending on part.numdims()
		af::array result = (sublevelComplement(
				convolveAF2(part,
						rotate(tool, float(i * 360.0 / n), true,
								AF_INTERP_BICUBIC_SPLINE), true), epsilon));
		result.as(f32);
		projectedContactCSpace += result;
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

	int d = part.numdims(); // d-dimensional part
	if (d == 2) {
		// Normalize the images;
		part /= 255.f;  // 3 channel RGB [0-1]
		tool /= 255.f;
	}

	getRotations(d);
	af::array piContactCSpace = computeProjectedContactCSpace(part, tool,
			epsilon);

	/*	if ((d == 2)){
	 do{
	 window.image(piContactCSpace);
	 } while( !window.close() );

	 }*/

}
