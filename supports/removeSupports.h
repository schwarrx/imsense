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

struct dislocationFeature{
	// Represent the features at which the tool
	// fractures the part geometry
	int featureid;
	// image indices corresponding to feature
	std::vector<int> indices;
};

void checkInputs(af::array nearNet, af::array tool);
std::vector<angleAxis> getRotations(int n); // sample rotations in SO(n)
void computeDislocationFeatures(af::array nearNet, af::array part);
af::array computeProjectedContactCSpace(af::array nearNet, af::array tool,
		std::vector<angleAxis> rotations, float epsilon); // compute contact space
void removeSupports(af::array nearNet, af::array tool,
		std::vector<angleAxis> rotations, float epsilon);

#endif /* REMOVESUPPORTS_H_ */
