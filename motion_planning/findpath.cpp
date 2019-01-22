/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * findpath.cpp
 *
 * Created on: Jan 15, 2019
 *     Author: saigopal nelaturi
 */

#include "findpath.h"
#include "helper.h"
#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/prm/PRM.h>
 #include <omplapp/apps/SE3MultiRigidBodyPlanning.h>

using namespace ompl;
namespace ob = ompl::base;
namespace og = ompl::geometric;

void findPath(std::string obstacles, std::string robot,
		std::vector<state> goal_states) {

	// Motion planning for a robot moving in SE(3)
	// in the presences of physical obstacles
	app::SE3MultiRigidBodyPlanning setup(goal_states.size());
	setup.setEnvironmentMesh(obstacles.c_str());
	setup.setRobotMesh(robot.c_str());
	// collision checker types are FCL and PQP
	// FCL appears to be better
	setup.setStateValidityCheckerType(ompl::app::FCL);

	// setting collision checking resolution to 1% of the space extent
	setup.getSpaceInformation()->setStateValidityCheckingResolution(0.01);

	// pick the planner
	setup.setPlanner(
			std::make_shared < og::RRTConnect > (setup.getSpaceInformation()));

	// define the state space as SE(3)
	ob::StateSpacePtr space(new ob::SE3StateSpace);

	// define compound start and goal states
	ob::ScopedState<base::CompoundStateSpace> start(
			setup.getSpaceInformation());
	ob::ScopedState<base::CompoundStateSpace> goal(setup.getSpaceInformation());

	for (std::vector<state>::iterator it = goal_states.begin();
			it != goal_states.end(); ++it) {

		cout << std::distance(goal_states.begin(), it) << endl;
		if (it != goal_states.end()) {
			auto *start_next = start->as<ob::SE3StateSpace::StateType>(
					std::distance(goal_states.begin(), it));
			start_next->setXYZ((*it).x, (*it).y, (*it).z);
			start_next->rotation().x = (*it).qx;
			start_next->rotation().y = (*it).qy;
			start_next->rotation().z = (*it).qz;
			start_next->rotation().w = (*it).qw;
		}

		if (it != goal_states.begin()) {
			auto *goal_next = goal->as<ob::SE3StateSpace::StateType>(
					std::distance(goal_states.begin(), it));
			goal_next->setXYZ((*it).x, (*it).y, (*it).z);
			goal_next->rotation().x = (*it).qx;
			goal_next->rotation().y = (*it).qy;
			goal_next->rotation().z = (*it).qz;
			goal_next->rotation().w = (*it).qw;
		}

	}

	// set start and goal states
	setup.setStartAndGoalStates(start, goal);

	// print setup info
	setup.setup();
	setup.print();

	// try to solve the motion planning problem
	if (setup.solve(20)) {
		// simplify & print the solution
		setup.simplifySolution();
		int num_states = 8;
		setup.getSolutionPath().interpolate(num_states);

		double length = setup.getSolutionPath().length();
		cout << "Path length =" << length << endl;

		setup.getSolutionPath().printAsMatrix(std::cout);
		// Get all the transformations in the path

	}

	//visualizePath(setup, obstacles, robot);
}
