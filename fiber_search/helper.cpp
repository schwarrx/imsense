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
#include <vtkSmoothPolyDataFilter.h>
#include <vtkCamera.h>
#include <vtkLight.h>
#include <vtkLightActor.h>

//Eigen
#include <Eigen/Geometry>

using namespace std;
using namespace ompl;
namespace ob = ompl::base;
namespace og = ompl::geometric;

void visualize(vtkSmartPointer<vtkAppendPolyData> unified) {
	// Visualize an unordered collection of parts in assembly

	vtkSmartPointer<vtkSmoothPolyDataFilter> smoothFilter = vtkSmartPointer<
			vtkSmoothPolyDataFilter>::New();
	smoothFilter->SetInputConnection(unified->GetOutputPort());
	smoothFilter->SetNumberOfIterations(15);
	smoothFilter->SetRelaxationFactor(0.1);
	smoothFilter->FeatureEdgeSmoothingOn();
	smoothFilter->BoundarySmoothingOn();
	smoothFilter->Update();

	// do the usual vtk rendering stuff
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<
			vtkPolyDataMapper>::New();
	mapper->SetInputConnection(unified->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetRepresentationToSurface();
	actor->GetProperty()->SetInterpolationToPhong();

	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(0, 0, 0.5);
	camera->SetFocalPoint(0.5, 0.5, 0.5);
	camera->SetViewUp(-0.612375, -0.612375, 0.50000);

	// create renderer
	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	ren->AddActor(actor);
	//ren->SetBackground(0.2, 0.5, 0.5);
	ren->SetBackground(1, 1, 2);
	ren->SetActiveCamera(camera);

	// add renderer to render window
	vtkSmartPointer<vtkRenderWindow> renWin =
			vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	renWin->SetSize(1400, 1400);

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

void visualizePath(app::SE3RigidBodyPlanning setup, std::string obstacles,
		std::string robot, std::string actual) {
	// first write to path
	// TODO -- read the states in memory without writing to file

	// FILE IO
	vtkSmartPointer<vtkSTLReader> obstacle_reader = readMesh(obstacles);
	vtkSmartPointer<vtkSTLReader> robot_reader = readMesh(robot);
	writePath(setup);

	// initialize the assembled states
	vtkSmartPointer<vtkAppendPolyData> appendTransformedRobot = vtkSmartPointer<
			vtkAppendPolyData>::New();

	// read path and transform states
	std::ifstream infile("file.txt");
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

	// add the actual part
	vtkSmartPointer<vtkSTLReader> actual_reader = readMesh(actual);
	appendTransformedRobot->AddInputConnection(actual_reader->GetOutputPort());

	visualize(appendTransformedRobot);
}

vtkSmartPointer<vtkActor> returnActor(
		vtkSmartPointer<vtkAppendPolyData> appended, double r, double g,
		double b) {

	vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<
			vtkPolyDataMapper>::New();
	mapper1->SetInputConnection(appended->GetOutputPort());

	vtkSmartPointer<vtkActor> actor1 = vtkSmartPointer<vtkActor>::New();
	actor1->SetMapper(mapper1);
	actor1->GetProperty()->SetInterpolationToPhong();
	actor1->GetProperty()->SetColor(r, g, b); //(R,G,B)

	return actor1;

}

void visualizeForPaper(app::SE3RigidBodyPlanning setup, std::string tool,
		std::string part, std::string supports, std::string platform) {

	// first read the supports & part stl file
	vtkSmartPointer<vtkSTLReader> partReader = readMesh(part);
	vtkSmartPointer<vtkSTLReader> toolReader = readMesh(tool);
	vtkSmartPointer<vtkSTLReader> supportsReader = readMesh(supports);
	vtkSmartPointer<vtkSTLReader> platformReader = readMesh(platform);

	writePath(setup);

	// create a vtk append polydata
	vtkSmartPointer<vtkAppendPolyData> appendPartAndPlatform = vtkSmartPointer<
			vtkAppendPolyData>::New();
	appendPartAndPlatform->AddInputConnection(platformReader->GetOutputPort());
	appendPartAndPlatform->AddInputConnection(partReader->GetOutputPort());

	// initialize the assembled states
	vtkSmartPointer<vtkAppendPolyData> appendTransformedRobot = vtkSmartPointer<
			vtkAppendPolyData>::New();

	// read path and transform states
	std::ifstream infile("file.txt");
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

		// add the 0.5 fudge factor (see goal + start state setting in findpath)
		const double rowmajor[16] = { rot(0, 0), rot(0, 1), rot(0, 2), x, rot(1,
				0), rot(1, 1), rot(1, 2), y + 0.5, rot(2, 0), rot(2, 1), rot(2,
				2), z, 0, 0, 0, 1 };
		// set up a vtk transform
		vtkSmartPointer<vtkTransform> transformation = vtkSmartPointer<
				vtkTransform>::New();
		transformation->SetMatrix(rowmajor);

		vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
				vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		transformFilter->SetInputConnection(toolReader->GetOutputPort());
		transformFilter->SetTransform(transformation);
		transformFilter->Update();

		appendTransformedRobot->AddInputConnection(
				transformFilter->GetOutputPort());

	}

	vtkSmartPointer<vtkActor> partAndPlatformActor = returnActor(
			appendPartAndPlatform, 0.2, 0.3, 0.7);

	vtkSmartPointer<vtkActor> sweptToolActor = returnActor(
			appendTransformedRobot, 0.7, 0.1, 0.1);

	// Now visualize the part+supports and the transformed robot
	// do the usual vtk rendering stuff
	vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<
			vtkPolyDataMapper>::New();
	mapper1->SetInputConnection(supportsReader->GetOutputPort());

	vtkSmartPointer<vtkActor> actor1 = vtkSmartPointer<vtkActor>::New();
	actor1->SetMapper(mapper1);
	actor1->GetProperty()->SetInterpolationToPhong();

	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(0, 0, 0.5);
	camera->SetFocalPoint(0.5, 0.5, 0.5);
	//camera->SetViewUp(-0.612375, -0.612375, 1);
	camera->SetViewUp(1, 1, 1);

	// create renderer
	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	ren->AddActor(partAndPlatformActor);
	ren->AddActor(sweptToolActor);
	ren->AddActor(actor1);
	//ren->SetBackground(0.2, 0.5, 0.5);
	ren->SetBackground(1, 1, 1);
	ren->SetActiveCamera(camera);

	// add renderer to render window
	vtkSmartPointer<vtkRenderWindow> renWin =
			vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	renWin->SetSize(1400, 1400);

	// create interactor
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<
			vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	// draw axes
	vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
	vtkSmartPointer<vtkOrientationMarkerWidget> widget = vtkSmartPointer<
			vtkOrientationMarkerWidget>::New();
	widget->SetOutlineColor(0.9300, 0.5700, 0.1300);
	//widget->SetOrientationMarker(axes);
	widget->SetInteractor(iren);
	widget->SetEnabled(1);
	widget->InteractiveOn();

	//render
	ren->ResetCamera();
	renWin->Render();
	iren->Start();

}

vtkSmartPointer<vtkActor> returnSTLActor(vtkSmartPointer<vtkSTLReader> appended,
		double r, double g, double b) {

	vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<
			vtkPolyDataMapper>::New();
	mapper1->SetInputConnection(appended->GetOutputPort());

	vtkSmartPointer<vtkActor> actor1 = vtkSmartPointer<vtkActor>::New();
	actor1->SetMapper(mapper1);
	actor1->GetProperty()->SetInterpolationToPhong();
	actor1->GetProperty()->SetColor(r, g, b); //(R,G,B)

	return actor1;

}

void visualizeSetupForPaper(std::string tool, std::string part,
		std::string supports, std::string platform, std::string disloc) {

	// first read the supports & part stl file
	vtkSmartPointer<vtkSTLReader> partReader = readMesh(part);
	vtkSmartPointer<vtkSTLReader> toolReader = readMesh(tool);
	vtkSmartPointer<vtkSTLReader> supportsReader = readMesh(supports);
	vtkSmartPointer<vtkSTLReader> platformReader = readMesh(platform);
	vtkSmartPointer<vtkSTLReader> dislocReader = readMesh(disloc);

	// create a vtk append polydata
	vtkSmartPointer<vtkAppendPolyData> appendPartAndPlatform = vtkSmartPointer<
			vtkAppendPolyData>::New();
	appendPartAndPlatform->AddInputConnection(platformReader->GetOutputPort());
	appendPartAndPlatform->AddInputConnection(partReader->GetOutputPort());

	vtkSmartPointer<vtkActor> partAndPlatformActor = returnActor(
			appendPartAndPlatform, 0.2, 0.3, 0.7);

	vtkSmartPointer<vtkActor> toolActor = returnSTLActor(toolReader, 0.7, 0.1,
			0.1);
	vtkSmartPointer<vtkActor> dislocActor = returnSTLActor(dislocReader, 0.1, 0.7,
			0.1);

	// Now visualize the part+supports and the transformed robot
	// do the usual vtk rendering stuff
	vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<
			vtkPolyDataMapper>::New();
	mapper1->SetInputConnection(supportsReader->GetOutputPort());

	vtkSmartPointer<vtkActor> actor1 = vtkSmartPointer<vtkActor>::New();
	actor1->SetMapper(mapper1);
	actor1->GetProperty()->SetInterpolationToPhong();
	actor1->GetProperty()->SetRepresentationToWireframe();

	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(0, 0, 0.5);
	camera->SetFocalPoint(0.5, 0.5, 0.5);
	//camera->SetViewUp(-0.612375, -0.612375, 1);
	camera->SetViewUp(1, 1, 1);

	// create renderer
	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	ren->AddActor(partAndPlatformActor);
	ren->AddActor(toolActor);
	ren->AddActor(dislocActor);
	ren->AddActor(actor1);
	//ren->SetBackground(0.2, 0.5, 0.5);
	ren->SetBackground(1, 1, 1);
	ren->SetActiveCamera(camera);

	// add renderer to render window
	vtkSmartPointer<vtkRenderWindow> renWin =
			vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	renWin->SetSize(1400, 1400);

	// create interactor
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<
			vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	// draw axes
	vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
	vtkSmartPointer<vtkOrientationMarkerWidget> widget = vtkSmartPointer<
			vtkOrientationMarkerWidget>::New();
	widget->SetOutlineColor(0.9300, 0.5700, 0.1300);
	//widget->SetOrientationMarker(axes);
	widget->SetInteractor(iren);
	widget->SetEnabled(1);
	widget->InteractiveOn();

	//render
	ren->ResetCamera();
	renWin->Render();
	iren->Start();

}
