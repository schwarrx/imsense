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
#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <string>

#include "ufabRV.h"
#include "helper.h"

using namespace std;



int main(int argc, char *argv[]) {
    try {

        if(argc !=4 ){
            cout << "usage: ./spatialTests partFile.binvox toolAssembly.binvox quaternionFile";
            exit(1);
        }
        // Select a device and display arrayfire info
        af::setDevice(6);
        af::info();

        int ndevices = getDeviceCount(); // number of available GPUs
        cout << "Reading rotation file ..";
        std::vector<Eigen::Matrix3d> rotationMatrices = getRotationMatricesFromFile(argv[3]);
        cout << "done" << endl;

        // part assembly indicator function
        af::array part = read_binvox(argv[1]);
        int partDim = part.dims()[0];
        //writeAFArray(part, "part.stl");
        //visualize(part);

        // tool assembly indicator function
        array toolAssembly = read_binvox(argv[2]);
        writeAFArray(rotate(toolAssembly,45, true, AF_INTERP_BICUBIC_SPLINE),"rotated45.stl");
        writeAFArray(reorder(toolAssembly, 2, 1, 0), "swapxz.stl");
        writeAFArray(rotate(reorder(toolAssembly, 2, 1, 0),45, true, AF_INTERP_BICUBIC_SPLINE),"swapxz_rotated45.stl");
        writeAFArray(reorder(toolAssembly, 0, 2, 1), "swapyz.stl");
        writeAFArray(rotate(reorder(toolAssembly, 0, 2, 1),45, true, AF_INTERP_BICUBIC_SPLINE),"swapxz_rotated45.stl");


        //writeAFArray(toolAssembly, "tool.stl");

        //assume tool assembly voxel resolution is tDim * tDim * tDim
        int tDim = toolAssembly.dims()[0];


        int resultDim = partDim + tDim -1;
        int n = 10; // 10 r-slices can be fit on a GPU
        af::array rSlices = array(resultDim, resultDim, resultDim, n);

        af::array projectedBoundary= constant(0,resultDim, resultDim,resultDim,f32);

        //omp_set_num_threads(ndevices-1);

        cout << "Part and tool dimensions = "<< partDim << "," << tDim << endl;
        cout << "starting " << endl;

        /*
        #pragma omp parallel
        {
        unsigned int cpu_thread_id = omp_get_thread_num();
        unsigned int num_cpu_threads = omp_get_num_threads();
         */

        af::timer::start();

        /*        for (int i = 0; i < ndevices ; i++)
        {*/

        /*        setDevice(cpu_thread_id % num_cpu_threads); // allows more CPU threads than GPU devices
        setDevice(i);
        cout << "CPU thread " << cpu_thread_id << " of " << num_cpu_threads << "  uses device " << getDevice() << endl;*/
        /////////////// NOTE : WE ARE ASSUMING THE SAME TOOL FOR EVERY ORIENTATION //////////////////

        gfor(seq i,n){
            af::array result = convolveAF(part, rotate(toolAssembly,45), true);
            rSlices(span,span,span,i) = result;
            projectedBoundary += result;
        }


        /*        }
        }*/
        cout << "Done computing in  " << af::timer::stop() << " s" << endl;
        //visualize(projectedBoundary);

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
 */
