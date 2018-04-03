/*
 * helper.h
 *
 *  Created on: March 20, 2017
 *      Author: nelaturi
 */

#ifndef HELPER_H_
#define HELPER_H_

#include <arrayfire.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>

#include <af/cuda.h>
#include <af/util.h>

struct angleAxis {
	// Represent rotations in angle axis format
	// Easier to handle both 2d and 3d this way.
	// For 3d it is recommended to convert to unit
	// quaternions.
	double angle;
	Eigen::Vector3d axis;
};

 
af::array read_binvox(std::string filespec);
void printGPUMemory();

double getAvailableDeviceMemory();
int getBatchSize(int d, int partDim, int toolDim, int resultDim);
void checkInputs(af::array nearNet, af::array tool, af::array part);
std::vector<angleAxis> getRotations(int d);

#endif /* HELPER_H_ */
