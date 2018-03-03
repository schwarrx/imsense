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
array maxRV (array x, array y, array infPocket);
array toolPlungeVolume(int length, int width, int depth);
array reflect(array x);
array convolveAF3(array x, array y, bool correlate);
array convolveAF2(array x, array y, bool correlate);
array levelSet(array x, double measure);

#endif /* CSPACEMORPH_H_ */
