/*
 * track.cpp
 *
 *  Created on: Nov 20, 2015
 *      Author: nelaturi
 */

#include "track.h"
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

// Global variables

int trackParticles(string input)
{
	try{

		// open the video file
		VideoCapture capture;
		if (input.empty())
			capture.open(0);
		else
			capture.open(input);
		if( !capture.isOpened() )
		{
			printf("\n Cannot open camera or video file\n");
			return -1;
		}

		Mat frame; //current frame
		Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
		Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
		int keyboard; //input from keyboard

		Mat tmpl;
		tmpl = imread("/home/nelaturi/imsense/code/afCudaTests/build/template.jpg");
		cvtColor(tmpl,tmpl,COLOR_GRAY2RGB);

		// define dimensions of a region of interest from the acquired frame

		//int frame_width=   capture.get(CV_CAP_PROP_FRAME_WIDTH);
		//int frame_height=   capture.get(CV_CAP_PROP_FRAME_HEIGHT);

		int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH) /4;
		int frame_height=   capture.get(CV_CAP_PROP_FRAME_HEIGHT)/2;

		cout<< "frame dimensions = " << frame_width << "x " << frame_height << endl;
		VideoWriter video("out.avi",CV_FOURCC('M','J','P','G'),10, Size(frame_width,frame_height),true);


		//create GUI windows
		namedWindow("Frame");
		namedWindow("FG Mask MOG 2");
		//create Background Subtractor objects
		pMOG2 = createBackgroundSubtractorMOG2(2,2); //MOG2 approach

		/*It is also a Gaussian Mixture-based
		Background/Foreground Segmentation Algorithm.
		It is based on two papers by Z.Zivkovic, "Improved adaptive Gausian mixture model for background subtraction"
		in 2004 and "Efficient Adaptive Density Estimation per Image Pixel for the Task of Background Subtraction" in 2006.
		One important feature of this algorithm is that it selects the appropriate number of gaussian distribution
		for each pixel. (Remember, in last case, we took a K gaussian distributions
		throughout the algorithm).
		It provides better adaptibility to varying scenes due illumination changes etc.*/

		//read input data. ESC or 'q' for quitting
		while( (char)keyboard != 'q' && (char)keyboard != 27 ){
			//read the current frame
			if(!capture.read(frame)) {
				cerr << "Unable to read next frame." << endl;
				cerr << "Exiting..." << endl;
				exit(EXIT_FAILURE);
			}

			Rect roi = Rect(0, frame_height, frame_width, frame_height);
			Mat frame_roi = frame(roi);

			//Mat res = TplMatch(frame_roi,tmpl);
			//imshow("example", frame_roi);
			// use arrayfire template matching

			//update the background model
			//std::clock_t start;
			//start = std::clock();
			pMOG2->apply(frame_roi, fgMaskMOG2);
			//double t = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
			//std::cout << "Time: " << t << " ms" << std::endl;
			//show the current frame and the fg masks

			int strel_type = MORPH_ELLIPSE; // structuring element type
			int strel_size = 3;
			Mat element = getStructuringElement( strel_type,
					Size( 2*strel_size + 1, 2*strel_size+1 ),
					Point( strel_size, strel_size ) );
			Mat dst;
			morphologyEx(fgMaskMOG2,dst,MORPH_OPEN,element);

			imshow("Frame", dst);

			//imshow("FG Mask MOG 2", strel_dst);
			Mat output;
			cvtColor(dst,output, COLOR_GRAY2RGB);
			video.write(output);
			//get the input from the keyboard
			keyboard = waitKey( 30 );

		}
		//delete capture object
		capture.release();
		//destroy GUI windows
		destroyAllWindows();
		return EXIT_SUCCESS;


		/*
		 * edge detection
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
		 */

	}
	catch( cv::Exception& e ){
		const char* err_msg = e.what();
		std::cout << "opencv exception caught: " << err_msg << std::endl;
		throw;
	}
	return 0;
}



af::array getChip(){
	// SN - this is a hack.
	af::array img_color = loadImage("template.jpg", true);
	af::array img = colorSpace(img_color, AF_GRAY, AF_RGB);
	return img;
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

af::array normalize(af::array a)
{
	float mx = af::max<float>(a);
	float mn = af::min<float>(a);
	return (a-mn)/(mx-mn);
}




point templateMatch(af::array img, af::array chip){
	// width/height = rectangle width/height
	// x,y = top left coordinates of bbox


	//time template matching
	std::clock_t start;
	start = std::clock();

	af::array result = matchTemplate(img,chip);
	double t = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
	//std::cout << "Time: " << t << " ms" << std::endl;
	af::array disp_res = normalize(result);
	unsigned minLocation;
	float    minVal;
	min<float>(&minVal, &minLocation, disp_res);

	dim4 iDims = img.dims();
	point res;
	res.x = minLocation%iDims[0];
	res.y = minLocation/iDims[0];

	return res;

}

int demoCDC(){
	std::string dir = string("/home/nelaturi/imsense/assembler/code/afCudaTests/frames/");
	std::string outdir = string("/home/nelaturi/imsense/assembler/code/afCudaTests/outframes/");

	// load template
	af::array img_tpl;
	img_tpl= loadImage("/home/nelaturi/imsense/assembler/code/afCudaTests/build/template.jpg", true);
	img_tpl= colorSpace(img_tpl, AF_GRAY, AF_RGB);

	vector<std::string> files = vector<std::string>();

	getdir(dir,files);
	files.erase(files.begin(), files.begin() + 2); // erase the . and .. files included by default
	std::sort(files.begin(), files.end(),numeric_string_compare);

	for (unsigned int i = 0;i < files.size();i++) {
		//cout << files[i] << endl;
		std::string fn = files[i];
		af::array img_color = loadImage((dir+fn).c_str(), true);
		dim4 iDims = img_color.dims();
		//cout << "height ="<< iDims[0] << ", width = " << iDims[1] << endl;
		unsigned int frame_width = iDims[1] /4;
		unsigned int frame_height= iDims[0] /2;
		af::array img_roi =img_color(seq(frame_height, 2*frame_height-100,1),seq(100,frame_width,1),1);
		// Convert the image from RGB to gray-scale
		// x increases along height, y increases along width
		af::array result = matchTemplate(img_roi,img_tpl);
		af::array disp_res = normalize(result);
		unsigned minLocation;
		float    minVal;
		min<float>(&minVal, &minLocation, disp_res);

		dim4 iDims1 = img_roi.dims();
		point res;
		res.x = minLocation%iDims1[0];
		res.y = minLocation/iDims1[1];
		cout << res.x +15 << "," << res.y +15 << endl;

		af::array disp_img = img_roi/255.0f; // normalize display image
		af::array marked_res = tile(disp_img, 1, 1, 3);
		//af::Window wnd("Template Matching Demo");


		drawRectangle(marked_res, 15+ res.x, 15+res.y, 3, 3);

		af::saveImage((outdir+fn).c_str(),marked_res);

		/*// Previews color image with green crosshairs
		if(!wnd.close()) {
			wnd.setColorMap(AF_COLORMAP_DEFAULT);
			wnd.grid(1, 2);
			wnd(0, 0).image(disp_img  , "Search Image"    );
			//wnd(0, 1).image(disp_tmp  , "Template Patch"  );
			wnd(0, 1).image(marked_res, "Best Match"      );
			wnd.setColorMap(AF_COLORMAP_HEAT);
			//wnd(1, 1).image(disp_res  , "Disparity values");
			wnd.show();
		}*/
	}
	return 1;
}

int getdir (std::string dir, vector<std::string> &files)
{
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(dir.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << dir << endl;
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		files.push_back(std::string(dirp->d_name));
	}
	closedir(dp);
	return 0;
}

bool numeric_string_compare(const std::string& s1, const std::string& s2)
{
	// handle empty strings...

	int a1 = atoi(s1.c_str());
	int a2 = atoi(s2.c_str());

	return (a1 < a2);
}
