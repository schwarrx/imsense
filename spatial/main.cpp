/*******************************************************
 * Author - S Nelaturi
 * Testing arrayfire ffts
 ********************************************************/

// Note : use cmake -DArrayFire_DIR=/home/nelaturi/arrayfire ..

#include <stdio.h>
#include <arrayfire.h>
#include <cstdio>
#include <cstdlib>

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
        std::cout << "Running fft tests for 3d convolution" << std::endl;


        array  x = read_binvox(argv[1]); // Part
        array  y = read_binvox(argv[2]); // Tool
        array y1 = read_binvox(argv[3]); // Cutting portion

        visualize(y);
        array maxrv;

        af::timer::start();
        maxrv = maxRV(x,y,y1);
    	cout << "Max removal volume (s):" <<  af::timer::stop() << endl;
    	cout << maxrv.dims() << endl;



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
