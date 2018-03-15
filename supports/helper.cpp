/*
 * helper.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: nelaturi
 */

#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <arrayfire.h>
#include "helper.h"

using namespace std;
using namespace af;

typedef unsigned char byte;
af::array read_binvox(string filespec) {
	// reads a binvox file
	static int version;
	static int depth, height, width;
	static int size;
	static byte *voxels = 0;
	static float tx, ty, tz;
	static float scale;

	ifstream *input = new ifstream(filespec.c_str(), ios::in | ios::binary);

	//
	// read header
	//
	string line;
	*input >> line;  // #binvox
	if (line.compare("#binvox") != 0) {
		cout << "Error: first line reads [" << line << "] instead of [#binvox]"
				<< endl;
		delete input;
		return 0;
	}
	*input >> version;
	//cout << "reading binvox version " << version << endl;

	depth = -1;
	int done = 0;
	while (input->good() && !done) {
		*input >> line;
		if (line.compare("data") == 0)
			done = 1;
		else if (line.compare("dim") == 0) {
			*input >> depth >> height >> width;
		} else if (line.compare("translate") == 0) {
			*input >> tx >> ty >> tz;
		} else if (line.compare("scale") == 0) {
			*input >> scale;
		} else {
			cout << "  unrecognized keyword [" << line << "], skipping" << endl;
			char c;
			do {  // skip until end of line
				c = input->get();
			} while (input->good() && (c != '\n'));

		}
	}
	if (!done) {
		cout << "  error reading header" << endl;
		return 0;
	}
	if (depth == -1) {
		cout << "  missing dimensions in header" << endl;
		return 0;
	}

	size = width * height * depth;
	voxels = new byte[size];
	if (!voxels) {
		cout << "  error allocating memory" << endl;
		return 0;
	}

	//
	// read voxel data
	//
	byte value;
	byte count;
	int index = 0;
	int end_index = 0;
	int nr_voxels = 0;

	input->unsetf(ios::skipws);  // need to read every byte now (!)
	*input >> value;  // read the linefeed char

	while ((end_index < size) && input->good()) {
		*input >> value >> count;

		if (input->good()) {
			end_index = index + count;
			if (end_index > size)
				return 0;
			for (int i = index; i < end_index; i++)
				voxels[i] = value;

			//cout << (float)value << endl;

			if (value)
				nr_voxels += count;
			index = end_index;
		}  // if file still ok

	}  // while

	input->close();

	af::array A = af::array(width, depth, height, voxels);
	A = A.as(f32);
	//cout << "read " << nr_voxels << " voxels" << endl;
	//cout << "Array dimensions = " << A.dims() << endl;
	//af_print(A);

	return A;

}

void printGPUMemory() {
	// show memory usage of GPU
	size_t free_byte;
	size_t total_byte;

	cudaError_t cuda_status = cudaMemGetInfo(&free_byte, &total_byte);

	if (cudaSuccess != cuda_status) {
		printf("Error: cudaMemGetInfo fails, %s \n",
				cudaGetErrorString(cuda_status));
		exit(1);
	}

	double free_db = (double) free_byte;
	double total_db = (double) total_byte;
	double used_db = total_db - free_db;
	//af::printMemInfo("af::printMemInfo ");
	printf("GPU memory usage: used = %f MB, free = %f MB, total = %f MB\n",
			used_db / 1024.0 / 1024.0, free_db / 1024.0 / 1024.0,
			total_db / 1024.0 / 1024.0);
}

double getAvailableDeviceMemory() {
	// get the available memory on GPU in bytes
	// show memory usage of GPU
	size_t free_byte;
	size_t total_byte;

	cudaError_t cuda_status = cudaMemGetInfo(&free_byte, &total_byte);

	if (cudaSuccess != cuda_status) {
		printf("Error: cudaMemGetInfo fails, %s \n",
				cudaGetErrorString(cuda_status));
		exit(1);
	}
	double free_db = static_cast<double>(free_byte);
	return free_db;

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

