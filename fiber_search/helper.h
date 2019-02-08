/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * helper.h
 *
 * Created on: Jan 15, 2019
 *     Author: saigopal nelaturi
 */

#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkSTLWriter.h>

#include <omplapp/apps/SE3RigidBodyPlanning.h>
#include <omplapp/apps/SE3MultiRigidBodyPlanning.h>
#include <omplapp/config.h>

void visualizePath(ompl::app::SE3RigidBodyPlanning setup, std::string obstacles,
		std::string robot, std::string actual);

void visualizeForPaper(ompl::app::SE3RigidBodyPlanning setup, std::string tool,
		std::string part, std::string supports, std::string platform);

void visualizeSetupForPaper(std::string tool, std::string part,
		std::string supports, std::string platform, std::string disloc);
