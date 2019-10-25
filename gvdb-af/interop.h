/*
 * interop.h
 *
 *  Created on: Jun 3, 2019
 *      Author: nelaturi
 */

#ifndef INTEROP_H_
#define INTEROP_H_


#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include <arrayfire.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace boost::filesystem;

typedef unsigned char byte;

af::array binvoxFile2AF(std::string filespec);

af::array imageStack2AF(const boost::filesystem::path &directory);


#endif /* INTEROP_H_ */
