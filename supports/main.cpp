/*******************************************************
 * Author - S Nelaturi
 * Testing arrayfire ffts
 ********************************************************/

// Note : use cmake -DArrayFire_DIR=/home/nelaturi/arrayfire ..
#include <stdio.h>
#include <arrayfire.h>
#include <assert.h>

#include "removeSupports.h"
#include "computeMaxFeasibleSet.h"
#include "helper.h"

using namespace std;

int main(int argc, char *argv[]) {
	try {
		if ((argc != 5)) {
			cout << "Number of arguments = " << argc << endl;
			cout << "usage = " << endl;
			cout
					<< "maximal set computation: ./analyzeCSpace obstaclesFile toolFile envelopeFile envelopeBoundaryFile \n"
					<< endl;
			//cout << "support removal ./analyzeCSpace nearNetFile toolFile partWithoutSupportsFile epsilon  \n" << endl;
			exit(1);
		}

		// Select a device and display arrayfire info
		af::setDevice(0);
		af::info();
		/*
		 if (argc == 5) {
		 // REMOVE SUPPORTS
		 cout << "Support removal" << endl;
		 // near net shape indicator function
		 af::array nearNet = af::loadImage(argv[1]);
		 nearNet.as(f32);
		 // tool assembly indicator function
		 af::array tool = af::loadImage(argv[2]);
		 tool.as(f32);
		 // goal part (without supports) indicator function
		 af::array part = af::loadImage(argv[3]);
		 part.as(f32);

		 // epsilon (tolerable overlap measure for contact)
		 float epsilon = atof(argv[4]);

		 int d = nearNet.numdims(); // d-dimensional part
		 if (d == 2) {
		 // Normalize the images;
		 nearNet /= 255.f;  // 3 channel RGB [0-1]
		 tool /= 255.f;
		 part /= 255.f;
		 }

		 runSupportRemoval(nearNet, tool, part, epsilon);
		 }*/

		if (argc == 5) {
			// COMPUTE MAXIMAL FEASIBLE SET
			cout << "Computing maximal feasible set" << endl;
			// physical obstacles indicator function
			cout << "loading obstacle set " << endl;
			af::array obstacles = af::loadImage(argv[1]);
			obstacles.as(f64);
			// tool assembly indicator function
			af::array tool = af::loadImage(argv[2]);
			cout << "loading tool assembly " << endl;
			tool.as(f64);
			af::array envelope = af::loadImage(argv[3]);
			cout << "loading envelope " << endl;
			envelope.as(f64);

			af::array envelopebd = af::loadImage(argv[4]);
			cout << "loading envelope bd" << endl;
			envelopebd.as(f64);

			int d = tool.numdims(); // d-dimensional part
			cout << "number of dimension = " << d << endl;
			if (d == 2) {
				// Normalize the images;
				tool /= 255.f;
				obstacles /= 255.f;
				envelope /= 255.f;
				envelopebd /= 255.f;
			}
			cout << "normalized images" << endl;

			//visualize2D(obstacles + envelopebd);
			af::saveImage("initialConstraints.png",
					complement(obstacles + envelopebd).as(f32));

			af::array maxFeas = maxFeasibleSet(obstacles, tool, envelope);

			af::array sublevelSet = sublevel(maxFeas, 40).as(f32);
			visualize2D(sublevelSet);
			af::saveImage("levelset.png", sublevelSet);


			writeImages(maxFeas, obstacles, envelope, envelopebd,tool);
		}

	} catch (af::exception& e) {
		fprintf(stderr, "%s\n", e.what());
		throw;
	}

#ifdef WIN32 // pause in Windows
	if (!(argc == 2 && argv[1][0] == '-')) {
		printf("hit [enter]...");
		fflush(stdout);
		getchar();
	}
#endif
	return 0;
}
