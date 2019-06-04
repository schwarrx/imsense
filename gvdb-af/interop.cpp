/*
 * interop.cpp
 *
 *  Created on: Jun 3, 2019
 *      Author: nelaturi
 */

#include "interop.h"

#include <fstream>
#include <iostream>


af::array binvoxFile2AF(std::string filespec) {

	// reads a binvox file
	static int version;
	static int depth, height, width;
	static int size;
	static byte *voxels = 0;
	static float tx, ty, tz;
	static float scale;

	std::ifstream *input = new std::ifstream(filespec.c_str(), std::ios::in | std::ios::binary);

	//
	// read header
	//
	std::string line;
	*input >> line;  // #binvox
	if (line.compare("#binvox") != 0) {
		std::cout << "Error: first line reads [" << line << "] instead of [#binvox]"
				<< std::endl;
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
			std::cout << "  unrecognized keyword [" << line << "], skipping" << std::endl;
			char c;
			do {  // skip until end of line
				c = input->get();
			} while (input->good() && (c != '\n'));

		}
	}
	if (!done) {
		std::cout << "  error reading header" << std::endl;
		return 0;
	}
	if (depth == -1) {
		std::cout << "  missing dimensions in header" << std::endl;
		return 0;
	}

	size = width * height * depth;
	voxels = new byte[size];
	if (!voxels) {
		std::cout << "  error allocating memory" << std::endl;
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

	input->unsetf(std::ios::skipws);  // need to read every byte now (!)
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
