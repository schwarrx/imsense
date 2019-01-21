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

using namespace ompl;
namespace ob = ompl::base;
namespace og = ompl::geometric;

/*
 *
 * Test with (from build directory)
 * ./findPath ../data/penny_bottle_opener.stl ../data/Entry_02_small.stl 20 -30 20 0 0 0 1 20 150 0 1 0 0 0
 *
 */
void findPath(std::string obstacles, std::string robot, state initial,
		state final) {

	// Motion planning for a robot moving in SE(3)
	// in the presences of physical obstacles
	app::SE3RigidBodyPlanning setup;
	setup.setEnvironmentMesh(obstacles.c_str());
	setup.setRobotMesh(robot.c_str());
	// collision checker types are FCL and PQP
	// FCL appears to be better
	setup.setStateValidityCheckerType(ompl::app::FCL);

	// setting collision checking resolution to 1% of the space extent
	setup.getSpaceInformation()->setStateValidityCheckingResolution(0.01);

	// pick the planner
	setup.setPlanner(
			std::make_shared < og::RRTConnect
					> (setup.getSpaceInformation()));

	// define the state space
	ob::StateSpacePtr space(new ob::SE3StateSpace);

	// define the start state for the robot
	ob::ScopedState<base::SE3StateSpace> start(setup.getSpaceInformation());
	start->setXYZ(initial.x, initial.y, initial.z);
	start->rotation().x = initial.qx;
	start->rotation().y = initial.qy;
	start->rotation().z = initial.qz;
	start->rotation().w = initial.qw;

	// define the goal state for the robot
	base::ScopedState<base::SE3StateSpace> goal(start);
	goal->setXYZ(final.x, final.y, final.z);
	goal->rotation().x = final.qx;
	goal->rotation().y = final.qy;
	goal->rotation().z = final.qz;
	goal->rotation().w = final.qw;

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

	visualizePath(setup, obstacles, robot);
}
