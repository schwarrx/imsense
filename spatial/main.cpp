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
#include <omp.h>

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

		int ndevices = getDeviceCount();

		std::cout << "Running tests for 3d removal volumes on " << ndevices-1 << " devices" << std::endl;


		// Find out how many parts can be copied on the gpu at once
		// allocate memory on the GPU
		// find out allocated memory on device


		//omp_set_num_threads(ndevices); 
		
//#pragma omp parallel
//{
		//unsigned int cpu_thread_id = omp_get_thread_num();
		//unsigned int num_cpu_threads = omp_get_num_threads();
		//for (int i = 0; i < ndevices ; i++)
		//{
		
		//setDevice(cpu_thread_id % num_cpu_threads); // allows more CPU threads than GPU devices
		//setDevice(i);
		//cout << "CPU thread " << cpu_thread_id << " of " << num_cpu_threads << "  uses device " << getDevice() << endl;
		
		/////////////// NOTE : WE ARE ASSUMING THE SAME TOOL FOR EVERY ORIENTATION //////////////////
		
		// #todo : describe n as a function of available memory
		int n = 60; // fit 85 tools per gpu

		// part assembly indicator function 
		array part = read_binvox(argv[1]);
		int partDim = part.dims()[0];

		// tool assembly indicator function
		array toolAssembly = read_binvox(argv[2]);
		//assume tool assembly voxel resolution is tDim * tDim * tDim
		int tDim = toolAssembly.dims()[0]; 
		
		cout << partDim << "," << tDim << endl;
		
		// calculate the maximal machinable volume resolution
		int rvDim = partDim + tDim -1;
		array removalVolumes = array(rvDim, rvDim, rvDim,n);
		
		// set the tool plunge volume as a function of length, width, depth of cut
		// this will result in an infinitesimal pocket
		array infPocket = toolPlungeVolume(5,5,5); // note - only cube masks supported in arrayfire cuda
		
		cout << "starting " << endl;
		
		af::timer::start();
		//for (int j = 0; j < n; j++){
		gfor (seq j,n){
		     removalVolumes(span,span,span,j) = maxRV(part, toolAssembly, infPocket); 
		     //maxRV(part, toolAssembly);
		} 
          	cout << "Done computing in  " << af::timer::stop() << " s" <<  endl;
 
		//}
		//visualize(removalVolumes(span,span,span,2));

//}


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
