/*
 * computeMaxFeasibleSet.h
 *
 *  Created on: Nov 29, 2018
 *      Author: nelaturi
 */

#ifndef COMPUTEMAXFEASIBLESET_H_
#define COMPUTEMAXFEASIBLESET_H_



#include "cspaceMorph.h"
#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <arrayfire.h>

using namespace std;

// compute the largest feasible set that the tool can reach
af::array maxFeasibleSet(af::array obstacles, af::array tool, af::array envelope);

// write images for visualization
void writeImages(af::array maxFeasible, af::array obstacles, af::array envelope, af::array envelopebd);


#endif /* COMPUTEMAXFEASIBLESET_H_ */
