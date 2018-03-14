/*
 * removeSupports.h
 *
 *  Created on: Mar 5, 2018
 *      Author: nelaturi
 */

#ifndef REMOVESUPPORTS_H_
#define REMOVESUPPORTS_H_

#include "cspaceMorph.h"
#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <arrayfire.h>

using namespace std;

struct angleAxis {
	// Represent rotations in angle axis format
	// Easier to handle both 2d and 3d this way.
	// For 3d it is recommended to convert to unit
	// quaternions.
	double angle;
	Eigen::Vector3d axis;
};

void checkInputs(af::array part, af::array tool);
std::vector<angleAxis> getRotations(int n); // sample rotations in SO(n)
af::array computeProjectedContactCSpace(af::array part, af::array tool,
		std::vector<angleAxis> rotations, float epsilon); // compute contact space
void removeSupports(af::array part, af::array tool,
		std::vector<angleAxis> rotations, float epsilon);

#endif /* REMOVESUPPORTS_H_ */
