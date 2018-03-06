/*
 * removeSupports.h
 *
 *  Created on: Mar 5, 2018
 *      Author: nelaturi
 */

#ifndef REMOVESUPPORTS_H_
#define REMOVESUPPORTS_H_

#include "cspaceMorph.h"

#include <arrayfire.h>

using namespace std;


void checkInputs(af::array part, af::array tool);
void getRotations(int n); // sample rotations in SO(n)
af::array computeProjectedContactCSpace(af::array part, af::array tool, float epsilon); // compute contact space

void removeSupports(af::array part, af::array tool, float epsilon);


#endif /* REMOVESUPPORTS_H_ */
