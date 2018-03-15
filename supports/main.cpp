/*******************************************************
 * Author - S Nelaturi
 * Testing arrayfire ffts
 ********************************************************/

// Note : use cmake -DArrayFire_DIR=/home/nelaturi/arrayfire ..
#include <stdio.h>
#include <arrayfire.h>
#include <assert.h>

#include "removeSupports.h"
#include "helper.h"

using namespace std;


int main(int argc, char *argv[]) {
	try {
	    cout << "Inputs are ok " << endl;

		if (argc != 5) {
			cout
					<< "usage: ./removeSupports nearNetFile toolFile partWithoutSupportsFile epsilon "
					<< endl;
			exit(1);
		}

		// Select a device and display arrayfire info
		af::setDevice(0);
		af::info();

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

/* Older code
 *
 *        // calculate the maximal machinable volume resolution
 int rvDim = partDim + tDim -1 ;

 int w = int(pow(2,ceil(log(rvDim)/log(2)))); // ensure it's a power of 2
 int h = w; int k = w; // assume (reasonably) that all dimensions in the conv are equal
 int d = tDim;
 cout << "rv dimensions = " << w << endl;

 array partExpanded(w,h,k);
 // copy the part voxels into the expanded array
 cout << (w-d)/2 << "," << (w-d)/2+ partDim << endl;
 af::seq seq1((w-d)/2, (w-d)/2+ partDim -1);
 partExpanded(seq1,seq1,seq1) = part;

 array toolExpanded(rvDim, rvDim, rvDim);
 af::seq seq2((w-d)/2, (w-d)/2+ tDim -1);
 toolExpanded(seq2,seq2,seq2) = toolAssembly;

 partExpanded = partExpanded.as(f32);
 toolExpanded = toolExpanded.as(f32);


 //writeAFArray(toolExpanded, "tool.stl");
 //writeAFArray(partExpanded, "part.stl");
 //throw std::exception();

 //array partNegative(rvDim, rvDim, rvDim);



 //}
 //visualize(removalVolumes(span,span,span,0));
 //visualize(removalVolume);
 // writeAFArray(removalVolume, "removal.stl")
 *
 *
 *



 //array removalVolumes = array(rvDim, rvDim, rvDim, n);

 // set the tool plunge volume as a function of length, width, depth of cut
 // this will result in an infinitesimal pocket
 //array infPocket = toolPlungeVolume(5, 5, 5); // note - only cube masks supported in arrayfire cuda



 //for (int j = 0; j < n; j++){
 //gfor (seq j,n)
 // {
 //array removalVolume = maxRV(partExpanded, toolExpanded,
 //    infPocket);
 //maxRV(part, toolAssembly);
 // }
 *
 *
 //        for (int i = 0; i < 360; i+=60){
 //            writeAFArray(rotate(toolAssembly,i, true, AF_INTERP_BICUBIC_SPLINE),"rotated" + std::to_string(i) +".stl");
 //            writeAFArray(rotate(toolAsmSwapXZ,i, true, AF_INTERP_BICUBIC_SPLINE),"rotatedSwapXZ" +std::to_string(i) + ".stl");
 //            writeAFArray(rotate(toolAsmSwapYZ,i, true, AF_INTERP_BICUBIC_SPLINE),"rotatedSwapYZ" + std::to_string(i)+".stl");
 //            writeAFArray(rotate(toolAsmSwapXY,i, true, AF_INTERP_BICUBIC_SPLINE),"rotatedSwapYZ" + std::to_string(i)+".stl");
 //
 //        }


 //writeAFArray(rotate(reorder(toolAssembly, 2, 1, 0),60, true, AF_INTERP_BICUBIC_SPLINE),"swapxz_rotated45.stl");
 //writeAFArray(reorder(toolAssembly, 0, 2, 1), "swapyz.stl");
 //writeAFArray(rotate(reorder(toolAssembly, 0, 2, 1),60, true, AF_INTERP_BICUBIC_SPLINE),"swapyz_rotated45.stl");
 //writeAFArray(toolAssembly, "tool.stl");

 *
 *
 *
 *
 */
