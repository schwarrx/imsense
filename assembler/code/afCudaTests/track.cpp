/*
 * track.cpp
 *
 *  Created on: Nov 20, 2015
 *      Author: nelaturi
 */

#include "track.h"


int trackParticles(const string& filename)
{
	try{
		VideoCapture cap(filename);

		 if(!cap.isOpened())  // check if we succeeded
		        return -1;

		    Mat edges;
		    namedWindow("edges",1);
		    for(;;)
		    {
		        Mat frame;
		        cap >> frame; // get a new frame from camera
		        cvtColor(frame, edges, CV_BGR2GRAY);
		        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
		        Canny(edges, edges, 0, 30, 3);
		        imshow("edges", edges);
		        if(waitKey(30) >= 0) break;
		    }


	}
	catch( cv::Exception& e ){
		const char* err_msg = e.what();
		std::cout << "opencv exception caught: " << err_msg << std::endl;
		throw;
	}
	return 0;
}

/*
af::array normalize(af::array a)
{
	float mx = af::max<float>(a);
	float mn = af::min<float>(a);
	return (a-mn)/(mx-mn);
}

void drawRectangle(af::array &out, unsigned x, unsigned y, unsigned dim0, unsigned dim1)
{
	//printf("\nMatching patch origin = (%u, %u)\n\n", x, y);
	seq col_span(x, x+dim0, 1);
	seq row_span(y, y+dim1, 1);
	//edge on left
	out(col_span, y       , 0) = 0.f;
	out(col_span, y       , 1) = 0.f;
	out(col_span, y       , 2) = 1.f;
	//edge on right
	out(col_span, y+dim1  , 0) = 0.f;
	out(col_span, y+dim1  , 1) = 0.f;
	out(col_span, y+dim1  , 2) = 1.f;
	//edge on top
	out(x       , row_span, 0) = 0.f;
	out(x       , row_span, 1) = 0.f;
	out(x       , row_span, 2) = 1.f;
	//edge on bottom
	out(x+dim0  , row_span, 0) = 0.f;
	out(x+dim0  , row_span, 1) = 0.f;
	out(x+dim0  , row_span, 2) = 1.f;
}


point templateMatch(af::array img, af::array chip, unsigned int width, unsigned int height, int x, int y, double &ct){
	// width/height = rectangle width/height
	// x,y = top left coordinates of bbox

	af::array region = img(seq(x, x+height, 1.0), seq(y, y+width, 1.0));
	//time template matching
	std::clock_t start;
	start = std::clock();
	af::array result_top_left = matchTemplate(region,chip);
	double t = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
	std::cout << "Time (top-left): " << t << " ms" << std::endl;
	ct +=t;
	af::array disp_res = normalize(result_top_left);
	unsigned minLocation;
	float    minVal;
	min<float>(&minVal, &minLocation, disp_res);

	dim4 iDims = region.dims();
	point result;
	result.x = x+ minLocation%iDims[0];
	result.y = y + minLocation/iDims[0];

	return result;

}*/
