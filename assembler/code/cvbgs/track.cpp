/*
 * track.cpp
 *
 *      Author: s nelaturi
 */

#include "track.h"
#include <opencv2/imgproc/imgproc.hpp>

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

		// define dimensions of a region of interest from the acquired frame

		//int frame_width=   capture.get(CV_CAP_PROP_FRAME_WIDTH);
		//int frame_height=   capture.get(CV_CAP_PROP_FRAME_HEIGHT);

		int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
		int frame_height=   capture.get(CV_CAP_PROP_FRAME_HEIGHT);

		cout<< "frame dimensions = " << frame_width << "x " << frame_height << endl;
		VideoWriter video("out.avi",CV_FOURCC('M','J','P','G'),10, Size(frame_width,frame_height),true);


		//create GUI windows
		namedWindow("Frame");
		namedWindow("FG Mask MOG 2");
		//create Background Subtractor objects
		//MOG2 (Mixture Of Gaussians) approach 2 - OpenCV has 2 implementations of mog
		pMOG2 = createBackgroundSubtractorMOG2(200,10);

		//read input data. ESC or 'q' for quitting
		while( (char)keyboard != 'q' && (char)keyboard != 27 ){
			//read the current frame
			if(!capture.read(frame)) {
				cerr << "Unable to read next frame." << endl;
				cerr << "Exiting..." << endl;
				exit(EXIT_FAILURE);
			}

			//std::clock_t start;
			//start = std::clock();
			pMOG2->apply(frame, fgMaskMOG2);
			//double t = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
			//std::cout << "Time: " << t << " ms" << std::endl;
			//show the current frame and the fg masks

			int strel_type = MORPH_ELLIPSE; // structuring element type
			int strel_size = 1;
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

	}
	catch( cv::Exception& e ){
		const char* err_msg = e.what();
		std::cout << "opencv exception caught: " << err_msg << std::endl;
		throw;
	}
	return 0;
}




