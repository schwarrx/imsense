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

struct dislocationFeature {
	// Represent the features at which the tool
	// fractures the part geometry
	int featureid;
	// image indices corresponding to feature
	std::vector<int> indices;
};


void runSupportRemoval(af::array nearNet, af::array tool,
		af::array part, float epsilon);


#endif /* REMOVESUPPORTS_H_ */
