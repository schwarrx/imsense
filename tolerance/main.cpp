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


	} catch (af::exception& e) {
		fprintf(stderr, "%s\n", e.what());
		throw;
	}

	return 0;
}
