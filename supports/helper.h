/*
 * helper.h
 *
 *  Created on: March 20, 2017
 *      Author: nelaturi
 */

#ifndef HELPER_H_
#define HELPER_H_

#include <arrayfire.h>
#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <string>
#include <fstream>
#include <iostream>

typedef unsigned char byte;
 
af::array read_binvox(std::string filespec); 
void visualize(af::array x);
void visualize2(af::array x, af::array y);

void writeAFArray(af::array x, std::string filename);
void saveVTK(std::string fileName, af::array outField);

std::vector<Eigen::Matrix3d> getRotationMatricesFromFile(const char* file);

#endif /* HELPER_H_ */
