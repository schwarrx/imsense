/*
 * track.h
 * 
 *      Author: s nelaturi
 */

#ifndef TRACK_H_
#define TRACK_H_

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>


#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

using namespace std;
using namespace cv;


int trackParticles(string input);


#endif /* TRACK_H_ */

