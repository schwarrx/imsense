/*
 * cspaceMorph.h
 *
 *  Created on: March 2, 2018
 *      Author: saigopal nelaturi
 */

#ifndef CSPACEMORPH_H_
#define CSPACEMORPH_H_

#include <arrayfire.h>
#include <fftw3.h>

using namespace af;

array indicator(array x);
array sublevel(array x, double measure);
array sublevelComplement(array x, double measure);
array convolveAF3(array x, array y, bool correlate);
array convolveAF2(array x, array y, bool correlate);
array levelSet(array x, double measure);
double volume(array x);
array complement(array x);

#endif /* CSPACEMORPH_H_ */
