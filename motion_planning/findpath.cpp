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

using namespace ompl;
namespace ob = ompl::base;
namespace og = ompl::geometric;

void findPath(std::string obstacles, std::string robot, state initial,
		state final) {

	// Motion planning for a robot moving in SE(3)
	// in the presences of physical obstacles

	app::SE3RigidBodyPlanning setup;
	setup.setEnvironmentMesh(obstacles.c_str());
	setup.setRobotMesh(robot.c_str());
	// define the state space
	ob::StateSpacePtr space(new ob::SE3StateSpace);

	// define the start state for the robot
	ob::ScopedState<base::SE3StateSpace> start(setup.getSpaceInformation());
	start->setXYZ(initial.x, initial.y, initial.z);
	start->rotation().setAxisAngle(initial.axis_x, initial.axis_y,
			initial.axis_z, initial.angle);

	// define the goal state for the robot
	base::ScopedState<base::SE3StateSpace> goal(start);
	goal->setXYZ(final.x, final.y, final.z);
	goal->rotation().setAxisAngle(final.axis_x, final.axis_y, final.axis_z,
			final.angle);

	// set start and goal states
	setup.setStartAndGoalStates(start, goal);

	// print setup info
	setup.setup();
	setup.print();

	// try to solve the motion planning problem
	if (setup.solve(10)) {
		// simplify & print the solution
		setup.simplifySolution();
		int num_states = 10;
		setup.getSolutionPath().interpolate(num_states);

		double length = setup.getSolutionPath().length();
		cout << "Path length =" << length << endl;

		setup.getSolutionPath().printAsMatrix(std::cout);
		// Get all the transformations in the path

	}

	visualizePath(setup, obstacles, robot);
}
