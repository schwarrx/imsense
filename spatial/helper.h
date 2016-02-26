/*
 * helper.h
 *
 *  Created on: Feb 24, 2016
 *      Author: nelaturi
 */

#ifndef HELPER_H_
#define HELPER_H_

#include <arrayfire.h>

typedef unsigned char byte;



af::array read_binvox(std::string filespec);
void visualize(af::array x);

#endif /* HELPER_H_ */
