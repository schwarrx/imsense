/*******************************************************
 * Author - S Nelaturi
 * Testing arrayfire ffts
 ********************************************************/

// Note : use cmake -DArrayFire_DIR=/home/nelaturi/arrayfire ..

#include <stdio.h>
#include <arrayfire.h>
#include <cstdio>
#include <cstdlib>
#include <assert.h>

#include "ufabRV.h"
#include "helper.h"

using namespace af;
using namespace std;

int main(int argc, char *argv[])
{
	try {
		// Select a device and display arrayfire info
		int device = argc > 1 ? atoi(argv[1]) : 0;
		af::setDevice(device);
		af::info();

		int ndevices = devicecount();

		std::cout << "Running tests for 3d removal volumes on " << ndevices << " devices" << std::endl;

		array x = read_binvox(argv[1]);
		cout << "Part dimensions = " << x.dims() << endl;

		// Find out how many parts can be copied on the gpu at once
		// allocate memory on the GPU
		// find out allocated memory on device
		//size_t bytes; size_t buffers; size_t lock_bytes; size_t lock_buffers;
		//deviceMemInfo(&bytes, &buffers, &lock_bytes, &lock_buffers);
		//cout << lock_bytes << endl;

		//array y = read_binvox(argv[2]); // Cutting portion


		array y1 = read_binvox(argv[3]); // Cutting portion
		cout << "Cutting surface dimensions = " <<  y1.dims() << endl;

		int n = 9;
		int part_dim = x.dims()[0];
		int tool_dim = 50; // 50 voxels per tool
		int rv_dim = part_dim + tool_dim-1;
		//cout << rv_dim << endl;

		vector<array> allTools;
		for (int i = 0; i < ndevices ; i++){
			deviceset(i);
			allTools.push_back(array(tool_dim, tool_dim, tool_dim, n));
		}

		array removal_vols = array(rv_dim, rv_dim, rv_dim,n);

		af::timer::start();
		for(int i = 0; i < ndevices ; i++){
			deviceset(i);
			gfor (seq j, n){
						allTools[i](span,span,span,j) = read_binvox(argv[2]);
			}
		}

		//cout << "Done allocating "  << n << " tool arrays on the GPU" << endl;/
		// allocate memory for max removal volume

		for(int i = 0 ; i < ndevices; i++){
			for (int j = 0; j < n; j++){
					//gfor(seq i,n){
			removal_vols(span,span,span,i) = maxRV(x,allTools[i](span,span,span,i),y1);
			}
		}
		cout << "Done computing in  " << af::timer::stop() << " s" <<  endl;


		//visualize(removal_vols(span,span,span,2));
		//array maxrv;


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
