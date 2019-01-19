/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * findpath.h
 *
 * Created on: Jan 15, 2019
 *     Author: saigopal nelaturi
 */

#ifndef PATH_H
#define PATH_H

#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkSTLWriter.h>

#include <iostream>
#include <omplapp/apps/SE3RigidBodyPlanning.h>
#include <omplapp/config.h>

struct state {
	// describes a configuration in SE(3)
	// translation components
	double x;
	double y;
	double z;
	// rotation components in axis angle format
	double axis_x;
	double axis_y;
	double axis_z;
	double angle; // in radians
};

// solve the motion planning problem
void findPath(std::string obstacles, std::string robot, state initial,
		state final);

#endif
