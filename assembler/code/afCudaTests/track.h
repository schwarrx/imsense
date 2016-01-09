/*
 * track.h
 *
 *  Created on: Nov 20, 2015
 *      Author: nelaturi
 */

#ifndef TRACK_H_
#define TRACK_H_

#include <cstdio>
#include <arrayfire.h>
#include <cstdlib>
#include <ctime>
#include <iostream>


#include "opencv2/opencv.hpp"

using namespace af;
using namespace std;
using namespace cv;

typedef struct{
	float x;
	float y;
}point ;

typedef struct{
	point topLeft;
	point topRight;
	point bottomLeft;
	point bottomRight;
}centers;

af::array normalize(af::array a);
void drawRectangle(af::array &out, unsigned x, unsigned y, unsigned dim0, unsigned dim1);

//location of minimum difference of abs values
point templateTRACK(af::array img, af::array chip, unsigned int width, unsigned int height, int x, int y, double &ct);


int trackParticles(const string& filename);




#endif /* TRACK_H_ */
