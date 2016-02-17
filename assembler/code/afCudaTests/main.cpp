/*******************************************************
 * Author - S Nelaturi
 * ArrayFire, OpenCV, and CUDA playground for A2P code
 ********************************************************/

// Note : use cmake -DArrayFire_DIR=/home/nelaturi/arrayfire ..

#include <stdio.h>
#include <arrayfire.h>
#include <iostream>


#include "track.h"

int main(int argc, char** argv)
{
	int device = argc > 1 ? atoi(argv[1]) : 0;

	try {
		af::setDevice(device);
		af::info();
		std::cout << "** ArrayFire, OpenCV, and CUDA tests for A2P  **" << std::endl;

		CommandLineParser parser(argc, argv, "{help h||}{@input||}");
		if (parser.has("help"))
		{
			std::cout << "usage: ./a2pcv-cuda videofile " << std::endl;
			return 0;
		}
		string input = parser.get<std::string>("@input");
		trackParticles(input);
		//demoCDC();

	} catch (af::exception& ae) {
		std::cerr << ae.what() << std::endl;
		throw;
	}

	return 0;
}


/*af::array getChip(){
	// SN - this is a hack. Extract the chip template from the first frame which is the cleanest
	// in the frames extracted to home/nelaturi/assembler/code/afCudaTests
	af::array img_color = loadImage("/home/nelaturi/assembler/code/afCudaTests/frames/1.jpg", true);
	af::array img = colorSpace(img_color, AF_GRAY, AF_RGB);
	unsigned patch_size = 40;
	int ystart = 588; int xstart = 170;
	af::array chip = img(seq(xstart, xstart+patch_size, 1.0), seq(ystart, ystart+patch_size, 1.0));
	return chip;
}




bool is_not_digit(char c)
{
	return !std::isdigit(c);
}

bool numeric_string_compare(const std::string& s1, const std::string& s2)
{
	// handle empty strings...

	int a1 = atoi(s1.c_str());
	int a2 = atoi(s2.c_str());

	return (a1 < a2);

}*/


/*

static void templateMatchingDemo()
{

	// get the chip template ( super hacky but whatever)
	af::array chip = getChip();

	// Load image
	af::array img_color;
	if (console)
		img_color = loadImage("/examples/images/square.png", true);
	else
		img_color = loadImage("/home/nelaturi/assembler/code/afCudaTests/frames/frame300.jpg", true);
	// Load all images from a directory

	std::string dir = string("/home/nelaturi/assembler/code/afCudaTests/frames/");
	std::string outdir = string("/home/nelaturi/assembler/code/afCudaTests/outframes/");

	vector<std::string> files = vector<std::string>();

	getdir(dir,files);
	files.erase(files.begin(), files.begin() + 2);
	std::sort(files.begin(), files.end(),numeric_string_compare);
	centers c ;

	double avg_time =0;
	for (unsigned int i = 0;i < files.size();i++) {
		//cout << files[i] << endl;
		std::string fn = files[i];
		af::array img_color = loadImage((dir+fn).c_str(), true);

		// Convert the image from RGB to gray-scale
		af::array img = colorSpace(img_color, AF_GRAY, AF_RGB);
		dim4 iDims = img.dims();
		//std::cout<<"Input image dimensions: " << iDims << std::endl << std::endl;
		// For visualization in af::arrayFire, color images must be in the [0.0f-1.0f] interval


		// manually define bounding rectangles for the first iteration
		// only global tracking for first frame
		unsigned width = 0;
		unsigned height = 0;
		double total_time = 0;
		// x increases along height, y increases along width

		int x = 0; int y = 0;
		//if(i==0){
			// spiral 1 ------(top left)
			x = 0; y =0; width = 650; height = 500;
			point loc1 = minLoc(img,chip,width,height,x,y, total_time);
			c.topLeft.x = loc1.x - 50;
			c.topLeft.y = loc1.y -50;
			//spiral 2 ----- (top right)
			x = 0; y = 650; width = 500; height = 500;
			point loc2 = minLoc(img,chip,width,height,x,y,total_time);
			c.topRight.x = loc2.x-50;
			c.topRight.y = loc2.y-50;
			//spiral 3 ---- (bottom left)
			x = 500; y = 0; width = 650; height = 400;
			point loc3 = minLoc(img,chip,width,height,x,y,total_time);
			c.bottomLeft.x = loc3.x-50;
			c.bottomLeft.y = loc3.y-50;
			//spiral 4 ---- (bottom right)
			x = 500; y = 650; width = 500; height = 400;
			point loc4 = minLoc(img,chip,width,height,x,y,total_time);
			c.bottomRight.x = loc4.x-50;
			c.bottomRight.y = loc4.y-50;

			af::array disp_img = img/255.0f; // normalize display image
			af::array marked_res = tile(disp_img, 1, 1, 3);

			unsigned int patch_size = 40; // this is the square dimension of the extracted chip, see getChip()
			if (!console) {
				// Draw a rectangle on input image where the template matches
				drawRectangle(marked_res, loc1.x, loc1.y, patch_size, patch_size);
				drawRectangle(marked_res, loc2.x, loc2.y, patch_size, patch_size);
				drawRectangle(marked_res, loc3.x, loc3.y, patch_size, patch_size);
				drawRectangle(marked_res, loc4.x, loc4.y, patch_size, patch_size);

				af::saveImage((outdir+fn).c_str(),marked_res);


		} else {
			//spiral 1 -- top left
			width = 700; height = 700;
			int ws = width/2; int hs = height/2;
			x = c.topLeft.x; y = c.topLeft.y;
			point loc1 = minLoc(img,chip,width,height,x,y, total_time);
			c.topLeft.x = loc1.x - hs;
			c.topLeft.y = loc1.y -ws;
			//spiral 2 ----- (top right)
			x = c.topRight.x; y = c.topRight.y;
			point loc2 = minLoc(img,chip,width,height,x,y,total_time);
			c.topRight.x = loc2.x-hs;
			c.topRight.y = loc2.y-ws;
			//spiral 3 ---- (bottom left)
			x = c.bottomLeft.x; y = c.bottomLeft.y;
			point loc3 = minLoc(img,chip,width,height,x,y,total_time);
			c.bottomLeft.x = loc3.x-hs;
			c.bottomLeft.y = loc3.y-ws;
			//spiral 4 ---- (bottom right)
			x = c.bottomRight.x; y = c.bottomRight.y;
			point loc4 = minLoc(img,chip,width,height,x,y,total_time);
			c.bottomRight.x = loc4.x-hs;
			c.bottomRight.y = loc4.y-ws;

			af::array disp_img = img/255.0f; // normalize display image
			af::array marked_res = tile(disp_img, 1, 1, 3);

			unsigned int patch_size = 40; // this is the square dimension of the extracted chip, see getChip()
			if (!console) {
				// Draw a rectangle on input image where the template matches

				drawRectangle(marked_res, loc1.x, loc1.y, patch_size, patch_size);
				drawRectangle(marked_res, loc2.x, loc2.y, patch_size, patch_size);
				drawRectangle(marked_res, loc3.x, loc3.y, patch_size, patch_size);
				drawRectangle(marked_res, loc4.x, loc4.y, patch_size, patch_size);

				af::saveImage((outdir+fn).c_str(),marked_res);

				std::cout<<"Note: Based on the disparity metric option provided to matchTemplate function\n"
								"either minimum or maximum disparity location is the starting corner\n"
								"of our best matching patch to template image in the search image"<< std::endl;

						af::Window wnd("Template Matching Demo");

						// Previews color image with green crosshairs
						if(!wnd.close()) {
							wnd.setColorMap(AF_COLORMAP_DEFAULT);
							wnd.grid(1, 2);
							wnd(0, 0).image(disp_img  , "Search Image"    );
							//wnd(0, 1).image(disp_tmp  , "Template Patch"  );
							wnd(0, 1).image(marked_res, "Best Match"      );
							wnd.setColorMap(AF_COLORMAP_HEAT);
							//wnd(1, 1).image(disp_res  , "Disparity values");
							wnd.show();
						}
			}


		}


		std::cout << "Total time = " << total_time << std::endl;

		avg_time += total_time;


		std::clock_t start;
		start = std::clock();

		af::array img_colorg = loadImage("/home/nelaturi/assembler/code/afCudaTests/frames/1.jpg", true);
		af::array imgg = colorSpace(img_colorg, AF_GRAY, AF_RGB);
	            // global template matching below
	                af::array result  = matchTemplate(imgg, chip); // Default disparity metric is
	                                                             // Sum of Absolute differences (SAD)
	                                                             // Currently supported metrics are
	                                                             // AF_SAD, AF_ZSAD, AF_LSAD, AF_SSD,
	                                                             // AF_ZSSD, ASF_LSSD


	        		double t = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
	        		std::cout << "global time = " << t <<  " for image with dimensions " << imgg.dims() << endl;


	}
	avg_time /= files.size();
	cout << "Average compute time = " << avg_time << endl;




}
*/



