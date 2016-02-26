/*
 * fftTests.h
 *
 *  Created on: Feb 24, 2016
 *      Author: saigopal nelaturi
 */

#ifndef FFTTESTS_H_
#define FFTTESTS_H_

#include <arrayfire.h>
#include <fftw3.h>

using namespace af;

array indicator(array x);
array sublevel(array x, double measure);
array maxRV(array x, array y, array y1);





#endif /* FFTTESTS_H_ */
