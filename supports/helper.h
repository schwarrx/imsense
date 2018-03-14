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

 
af::array read_binvox(std::string filespec);
void printGPUMemory();

double getAvailableDeviceMemory();


#endif /* HELPER_H_ */
