/*
 * helper.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: nelaturi
 */

#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include <arrayfire.h>

#include "helper.h"
// vtk stuff
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolumeProperty.h>

#include <vtkSTLWriter.h>

using namespace std;
using namespace af;

af::array read_binvox(string filespec) {
    // reads a binvox file
    static int version;
    static int depth, height, width;
    static int size;
    static byte *voxels = 0;
    static float tx, ty, tz;
    static float scale;

    ifstream *input = new ifstream(filespec.c_str(), ios::in | ios::binary);

    //
    // read header
    //
    string line;
    *input >> line;  // #binvox
    if (line.compare("#binvox") != 0) {
        cout << "Error: first line reads [" << line << "] instead of [#binvox]"
                << endl;
        delete input;
        return 0;
    }
    *input >> version;
    //cout << "reading binvox version " << version << endl;

    depth = -1;
    int done = 0;
    while (input->good() && !done) {
        *input >> line;
        if (line.compare("data") == 0)
            done = 1;
        else if (line.compare("dim") == 0) {
            *input >> depth >> height >> width;
        } else if (line.compare("translate") == 0) {
            *input >> tx >> ty >> tz;
        } else if (line.compare("scale") == 0) {
            *input >> scale;
        } else {
            cout << "  unrecognized keyword [" << line << "], skipping" << endl;
            char c;
            do {  // skip until end of line
                c = input->get();
            } while (input->good() && (c != '\n'));

        }
    }
    if (!done) {
        cout << "  error reading header" << endl;
        return 0;
    }
    if (depth == -1) {
        cout << "  missing dimensions in header" << endl;
        return 0;
    }

    size = width * height * depth;
    voxels = new byte[size];
    if (!voxels) {
        cout << "  error allocating memory" << endl;
        return 0;
    }

    //
    // read voxel data
    //
    byte value;
    byte count;
    int index = 0;
    int end_index = 0;
    int nr_voxels = 0;

    input->unsetf(ios::skipws);  // need to read every byte now (!)
    *input >> value;  // read the linefeed char

    while ((end_index < size) && input->good()) {
        *input >> value >> count;

        if (input->good()) {
            end_index = index + count;
            if (end_index > size)
                return 0;
            for (int i = index; i < end_index; i++)
                voxels[i] = value;

            //cout << (float)value << endl;

            if (value)
                nr_voxels += count;
            index = end_index;
        }  // if file still ok

    }  // while

    input->close();

    af::array A = af::array(width, depth, height, voxels);
    A = A.as(f32);
    //cout << "read " << nr_voxels << " voxels" << endl;
    //cout << "Array dimensions = " << A.dims() << endl;
    //af_print(A);

    return A;

}

/*
 __global__ void transform_tex(float *dest, int w, int h, int d,  float x11, float x12, float x13,
 float x21, float x22, float x23,
 float x31, float x32, float x33,
 float tx, float ty, float tz)
 {
 int x_i= threadIdx.x;
 int y_i= blockIdx.x;
 int z_i= blockIdx.y;


 // Convert to cartesian coordinates - swap x and z
 float z = x_i;
 float y = y_i;
 float x = z_i;

 // Normalize coordinates, also consider only center of voxels
 // Transform grid from (0,0,0)-(1,1,1) to (-0.5,-0.5,-0.5)- (0.5,0.5,0.5)
 // This way (0,0,0) is in the center of the grid
 x = (x+0.5)/w -0.5;
 y = (y+0.5)/h -0.5;
 z = (z+0.5)/d -0.5;



 tx = tx/w;
 ty = ty/h;
 tz = tz/d;


 // Rotate the coordinates (inverse transformation )
 // idea is to transform the coordinates of the voxel grid by the inverse transformation to map to points on the
 // original grid from which an interpolated value can be computed
 float sx = x11*(x-tx)+ x21*(y-ty) + x31*(z-tz);
 float sy = x12*(x-tx)+ x22*(y-ty) + x32*(z-tz);
 float sz = x13*(x-tx)+ x23*(y-ty) + x33*(z-tz);

 // Translate and scale back to (0,0,0)-(w,h,d)
 sx = (sx + 0.5)*w;
 sy = (sy + 0.5)*h;
 sz = (sz + 0.5)*d;

 // Get the correct coordinates in the array - swap sx and sz
 float temp = sx;
 sx = sz; sz = temp;

 if((sx <0) || (sy < 0) || (sz <0) || (sx >=w) || (sy >=h) || (sz >=d))
 return;

 int ix= y_i*w + x_i;
 dest[(z_i*w*h)+ix] = tex3D(mtx_tex,sx,sy,sz);
 }

 */

void writeSTL(vtkSmartPointer<vtkPolyData> polyData, std::string filename) {
    // write a polydata out to stl

    vtkSmartPointer<vtkSTLWriter> stlWriter =
            vtkSmartPointer<vtkSTLWriter>::New();
    stlWriter->SetFileName(filename.c_str());
    stlWriter->SetInputData(polyData);
    stlWriter->Write();

}

void writeAFArray(array x, std::string filename) {
    // copy data from device to host and visualize on vtk
    // not sure how to do this visualization directly in AF
    array x1 = x.as(f64);
    double *host_x = x1.host<double>(); // copy from device to host - expensive
    dim4 dim = x.dims();
    //create image data
    vtkSmartPointer<vtkImageData> imageData =
            vtkSmartPointer<vtkImageData>::New();
    //specify size of image data
    imageData->SetDimensions(dim[0], dim[1], dim[2]);
#if VTK_MAJOR_VERSION <= 5
    imageData->SetNumberOfScalarComponents(1);
    imageData->SetScalarTypeToDouble();
#else
    imageData->AllocateScalars(VTK_DOUBLE, 1);
#endif
    //populate imageData array
    cout << "Copying to imageData and writing" << endl;
    for (int k = 0; k < dim[2]; k++) {
        for (int j = 0; j < dim[1]; j++) {
            for (int i = 0; i < dim[0]; i++) {
                double *voxel =
                        static_cast<double*>(imageData->GetScalarPointer(i, j,
                                k));
                voxel[0] = host_x[j * dim[0] * dim[1] + i * dim[1] + k];
            }
        }
    }

    // Create a 3D model using marching cubes
    vtkSmartPointer<vtkMarchingCubes> mc =
            vtkSmartPointer<vtkMarchingCubes>::New();
    mc->SetInputData(imageData);
    mc->ComputeNormalsOn();
    mc->ComputeGradientsOn();
    mc->SetValue(0, 1);  // second value acts as threshold
    mc->Update();

    writeSTL(mc->GetOutput(), filename);

}

void visualize(array x) {
    // copy data from device to host and visualize on vtk
    // not sure how to do this visualization directly in AF
    array x1 = x.as(f64);
    double *host_x = x1.host<double>(); // copy from device to host - expensive
    dim4 dim = x.dims();
    //create image data
    vtkSmartPointer<vtkImageData> imageData =
            vtkSmartPointer<vtkImageData>::New();
    //specify size of image data
    imageData->SetDimensions(dim[0], dim[1], dim[2]);
#if VTK_MAJOR_VERSION <= 5
    imageData->SetNumberOfScalarComponents(1);
    imageData->SetScalarTypeToDouble();
#else
    imageData->AllocateScalars(VTK_DOUBLE, 1);
#endif
    //populate imageData array
    cout << "Copying to imageData and visualizing" << endl;
    for (int k = 0; k < dim[2]; k++) {
        for (int j = 0; j < dim[1]; j++) {
            for (int i = 0; i < dim[0]; i++) {
                double *voxel =
                        static_cast<double*>(imageData->GetScalarPointer(i, j,
                                k));
                voxel[0] = host_x[j * dim[0] * dim[1] + i * dim[1] + k];
            }
        }
    }

    // Create a 3D model using marching cubes
    vtkSmartPointer<vtkMarchingCubes> mc =
            vtkSmartPointer<vtkMarchingCubes>::New();
    mc->SetInputData(imageData);
    mc->ComputeNormalsOn();
    mc->ComputeGradientsOn();
    mc->SetValue(0, 1);  // second value acts as threshold

    // To remain largest region
    vtkSmartPointer<vtkPolyDataConnectivityFilter> confilter = vtkSmartPointer<
            vtkPolyDataConnectivityFilter>::New();
    confilter->SetInputConnection(mc->GetOutputPort());
    confilter->SetExtractionModeToLargestRegion();

    bool extractMaxIsoSurface = false;
    // Create a mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<
            vtkPolyDataMapper>::New();
    if (extractMaxIsoSurface) {
        mapper->SetInputConnection(confilter->GetOutputPort());
    } else {
        mapper->SetInputConnection(mc->GetOutputPort());
    }

    mapper->ScalarVisibilityOff();    // utilize actor's property I set

    // Visualize
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->GetProperty()->SetColor(1, 1, 1);
    actor->SetMapper(mapper);

    vtkSmartPointer<vtkRenderWindow> renWin =
            vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderer> ren1 = vtkSmartPointer<vtkRenderer>::New();
    ren1->SetBackground(0.1, 0.4, 0.2);

    ren1->AddViewProp(actor);
    ren1->ResetCamera();
    renWin->AddRenderer(ren1);
    renWin->SetSize(301, 300); // intentional odd and NPOT  width/height

    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<
            vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renWin);

    renWin->Render(); // make sure we have an OpenGL context.
    iren->Start();

}


void visualize2(array x, array y) {
    // copy data from device to host and visualize on vtk
    // not sure how to do this visualization directly in AF
    array x1 = x.as(f64);
    double *host_x = x1.host<double>(); // copy from device to host - expensive

    array y1 = y.as(f64);
    double *host_y = y1.host<double>(); // copy from device to host - expensive


    dim4 dim = x.dims();
    dim4 ydim = y.dims();

    //create image data
    vtkSmartPointer<vtkImageData> imageData =
            vtkSmartPointer<vtkImageData>::New();
    //specify size of image data
    imageData->SetDimensions(dim[0], dim[1], dim[2]);
#if VTK_MAJOR_VERSION <= 5
    imageData->SetNumberOfScalarComponents(1);
    imageData->SetScalarTypeToDouble();
#else
    imageData->AllocateScalars(VTK_DOUBLE, 1);
#endif
    //populate imageData array
    cout << "Copying to imageData and visualizing" << endl;
    for (int k = 0; k < dim[2]; k++) {
        for (int j = 0; j < dim[1]; j++) {
            for (int i = 0; i < dim[0]; i++) {
                double *voxel =
                        static_cast<double*>(imageData->GetScalarPointer(i, j,
                                k));
                voxel[0] = host_x[j * dim[0] * dim[1] + i * dim[1] + k];
            }
        }
    }

    //create image data
     vtkSmartPointer<vtkImageData> imageData2 =
             vtkSmartPointer<vtkImageData>::New();
     //specify size of image data
     imageData->SetDimensions(ydim[0], ydim[1], ydim[2]);
 #if VTK_MAJOR_VERSION <= 5
     imageData2->SetNumberOfScalarComponents(1);
     imageData2->SetScalarTypeToDouble();
 #else
     imageData2->AllocateScalars(VTK_DOUBLE, 1);
 #endif
     //populate imageData array
     cout << "Copying to imageData 2 and visualizing" << endl;
     for (int k = 0; k < ydim[2]; k++) {
         for (int j = 0; j < ydim[1]; j++) {
             for (int i = 0; i < ydim[0]; i++) {
                 double *voxel =
                         static_cast<double*>(imageData2->GetScalarPointer(i, j,
                                 k));
                 voxel[0] = host_y[j * dim[0] * dim[1] + i * dim[1] + k];
             }
         }
     }
    // Create a 3D model using marching cubes -- for X
    vtkSmartPointer<vtkMarchingCubes> mc =
            vtkSmartPointer<vtkMarchingCubes>::New();
    mc->SetInputData(imageData);
    mc->ComputeNormalsOn();
    mc->ComputeGradientsOn();
    mc->SetValue(0, 1);  // second value acts as threshold

    // To remain largest region
    vtkSmartPointer<vtkPolyDataConnectivityFilter> confilter = vtkSmartPointer<
            vtkPolyDataConnectivityFilter>::New();
    confilter->SetInputConnection(mc->GetOutputPort());
    confilter->SetExtractionModeToLargestRegion();


    // Create a 3D model using marching cubes
    vtkSmartPointer<vtkMarchingCubes> mc2 =
            vtkSmartPointer<vtkMarchingCubes>::New();
    mc2->SetInputData(imageData2);
    mc2->ComputeNormalsOn();
    mc2->ComputeGradientsOn();
    mc2->SetValue(0, 1);  // second value acts as threshold

    // To remain largest region
    vtkSmartPointer<vtkPolyDataConnectivityFilter> confilter2 = vtkSmartPointer<
            vtkPolyDataConnectivityFilter>::New();
    confilter2->SetInputConnection(mc2->GetOutputPort());
    confilter2->SetExtractionModeToLargestRegion();

    bool extractMaxIsoSurface = false;
    // Create a mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<
            vtkPolyDataMapper>::New();
    if (extractMaxIsoSurface) {
        mapper->SetInputConnection(confilter->GetOutputPort());
    } else {
        mapper->SetInputConnection(mc->GetOutputPort());
    }

    mapper->ScalarVisibilityOff();    // utilize actor's property I set

    // Create a mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<
            vtkPolyDataMapper>::New();
    if (extractMaxIsoSurface) {
        mapper2->SetInputConnection(confilter2->GetOutputPort());
    } else {
        mapper2->SetInputConnection(mc2->GetOutputPort());
    }

    mapper->ScalarVisibilityOff();    // utilize actor's property I set

    // Visualize
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->GetProperty()->SetColor(1, 1, 1);
    actor->SetMapper(mapper);

    // Visualize
    vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkActor>::New();
    actor2->GetProperty()->SetColor(1, 1, 1);
    actor2->SetMapper(mapper);

    vtkSmartPointer<vtkRenderWindow> renWin =
            vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderer> ren1 = vtkSmartPointer<vtkRenderer>::New();
    ren1->SetBackground(0.1, 0.4, 0.2);

    ren1->AddViewProp(actor);
    ren1->AddViewProp(actor2);
    ren1->ResetCamera();
    renWin->AddRenderer(ren1);
    renWin->SetSize(301, 300); // intentional odd and NPOT  width/height

    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<
            vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renWin);

    renWin->Render(); // make sure we have an OpenGL context.
    iren->Start();

}

