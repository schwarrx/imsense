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

		std::cout << "Running tests for 3d removal volumes on " << ndevices << " devices" << std::endl;


		// Find out how many parts can be copied on the gpu at once
		// allocate memory on the GPU
		// find out allocated memory on device

		//array y = read_binvox(argv[2]); // Cutting portion

		omp_set_num_threads(ndevices-1);

#pragma omp parallel
{
		unsigned int cpu_thread_id = omp_get_thread_num();
		unsigned int num_cpu_threads = omp_get_num_threads();
		for (int i = 0; i < ndevices-1 ; i++)
		{
		setDevice(cpu_thread_id % num_cpu_threads); // allows more CPU threads than GPU devices
		//setDevice(i);
		cout << "CPU thread " << cpu_thread_id << " of " << num_cpu_threads << "  uses device " << getDevice() << endl;
		int n = 25; // fit 25 tools per gpu

		array x = read_binvox(argv[1]);
		int x_dim = x.dims()[0];


		int t_dim = 50; // 50 voxels per tool
		int rv_dim = x_dim + t_dim-1;
		array y1 = read_binvox(argv[3]); // Cutting portion
		//cout << "Part dimensions = " << x.dims() << endl;
		//cout << "Cutting surface dimensions = " <<  y1.dims() << endl;
        array removal_vols = array(rv_dim, rv_dim, rv_dim,n);
		array allTools = array(t_dim, t_dim, t_dim, n);
		af::timer::start();
		gfor (seq j, n){
		     allTools(span,span,span,j) = read_binvox(argv[2]);
		}
		//gfor (seq j,n){
		for (int j = 0; j < n; j++){
		     //removal_vols(span,span,span,j) = maxRV(x,allTools(span,span,span,j),y1);
		     maxRV(x,allTools(span,span,span,j),y1);
		}
		deviceGC();
          	cout << "Done computing in  " << af::timer::stop() << " s" <<  endl;

		}

}

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
