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
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

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

int trackParticles(string input);


//convert opencv Mat to arrayfire array
void mat2array(cv::Mat& input, array& output);

point templateMatch(af::array img, af::array chip);

int demoCDC();
int getdir (std::string dir, vector<std::string> &files);
bool numeric_string_compare(const std::string& s1, const std::string& s2);

#endif /* TRACK_H_ */
