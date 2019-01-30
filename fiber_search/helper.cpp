/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * helper.cpp
 *
 * Created on: Jan 15, 2019
 *     Author: saigopal nelaturi
 */

#include "helper.h"
#include <fstream>
#include <sstream>
#include <string>

// VTK stuff
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkMatrix4x4.h>
#include <vtkSTLReader.h>
#include <vtkAppendPolyData.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>

//Eigen
#include <Eigen/Geometry>

using namespace std;
using namespace ompl;
namespace ob = ompl::base;
namespace og = ompl::geometric;

void visualize(vtkSmartPointer<vtkAppendPolyData> unified) {
	// Visualize an unordered collection of parts in assembly

	// do the usual vtk rendering stuff
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<
			vtkPolyDataMapper>::New();
	mapper->SetInputConnection(unified->GetOutputPort());
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// create renderer
	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	ren->AddActor(actor);
	ren->SetBackground(0.2, 0.5, 0.5);

	// add renderer to render window
	vtkSmartPointer<vtkRenderWindow> renWin =
			vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	renWin->SetSize(400, 400);

	// create interactor
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<
			vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	// draw axes
	vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
	vtkSmartPointer<vtkOrientationMarkerWidget> widget = vtkSmartPointer<
			vtkOrientationMarkerWidget>::New();
	widget->SetOutlineColor(0.9300, 0.5700, 0.1300);
	widget->SetOrientationMarker(axes);
	widget->SetInteractor(iren);
	widget->SetEnabled(1);
	widget->InteractiveOn();

	//render
	ren->ResetCamera();
	renWin->Render();
	iren->Start();

}

void stlWrite(string filename, vtkSmartPointer<vtkPolyData> polydata) {
	vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
	writer->SetFileName(filename.c_str());
	writer->SetInputData(polydata);
	writer->SetFileTypeToBinary();
	writer->Write();

}

void writePath(app::SE3RigidBodyPlanning setup) {
	// writes a path to file.txt
	std::ofstream out("file.txt");
	setup.getSolutionPath().printAsMatrix(out);
}

vtkSmartPointer<vtkSTLReader> readMesh(std::string filename) {
	vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName(filename.c_str());
	reader->Update();
	return reader;
}

void visualizePath( std::string obstacles,
		std::string robot, const char* filename) {
	// first write to path
	// TODO -- read the states in memory without writing to file

	// FILE IO
	vtkSmartPointer<vtkSTLReader> obstacle_reader = readMesh(obstacles);
	vtkSmartPointer<vtkSTLReader> robot_reader = readMesh(robot);

	// initialize the assembled states
	vtkSmartPointer<vtkAppendPolyData> appendTransformedRobot = vtkSmartPointer<
			vtkAppendPolyData>::New();

	// read path and transform states
	std::ifstream infile(filename);
	std::string line;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);
		double x, y, z, qx, qy, qz, qw;
		if (!(iss >> x >> y >> z >> qx >> qy >> qz >> qw)) {
			break;
		} // error

		Eigen::Quaterniond q;
		q.x() = qx;
		q.y() = qy;
		q.z() = qz;
		q.w() = qw;
		q.normalize();
		Eigen::Matrix3d rot = q.toRotationMatrix();

		const double rowmajor[16] = { rot(0, 0), rot(0, 1), rot(0, 2), x, rot(1,
				0), rot(1, 1), rot(1, 2), y, rot(2, 0), rot(2, 1), rot(2, 2), z,
				0, 0, 0, 1 };
		// set up a vtk transform
		vtkSmartPointer<vtkTransform> transformation = vtkSmartPointer<
				vtkTransform>::New();
		transformation->SetMatrix(rowmajor);

		vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
				vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		transformFilter->SetInputConnection(robot_reader->GetOutputPort());
		transformFilter->SetTransform(transformation);
		transformFilter->Update();

		appendTransformedRobot->AddInputConnection(
				transformFilter->GetOutputPort());

	}
	// add the obstacle
	appendTransformedRobot->AddInputConnection(
			obstacle_reader->GetOutputPort());

	visualize(appendTransformedRobot);
}
