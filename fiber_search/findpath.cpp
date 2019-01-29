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
#include "se3graph.h"
#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/prm/PRM.h>
#include <iterator>
#include <vector>

using namespace ompl;
namespace ob = ompl::base;
namespace og = ompl::geometric;

void findPath(std::string obstacles, std::string robot,
		std::vector<state> goal_states) {

	// Motion planning for a robot moving in SE(3)
	// in the presences of physical obstacles
	app::SE3MultiRigidBodyPlanning setup(goal_states.size() - 1);
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

	for (auto it = goal_states.begin(); it != std::prev(goal_states.end());
			++it) {

		auto *start_next = start->as<ob::SE3StateSpace::StateType>(
				std::distance(goal_states.begin(), it));
		start_next->setXYZ((*it).x, (*it).y, (*it).z);
		start_next->rotation().x = (*it).qx;
		start_next->rotation().y = (*it).qy;
		start_next->rotation().z = (*it).qz;
		start_next->rotation().w = (*it).qw;

		auto *goal_next = goal->as<ob::SE3StateSpace::StateType>(
				std::distance(goal_states.begin(), it));
		std::vector<state>::iterator nxt = std::next(it);
		goal_next->setXYZ((*nxt).x, (*nxt).y, (*nxt).z);
		goal_next->rotation().x = (*nxt).qx;
		goal_next->rotation().y = (*nxt).qy;
		goal_next->rotation().z = (*nxt).qz;
		goal_next->rotation().w = (*nxt).qw;

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
		int num_states = 10;
		setup.getSolutionPath().interpolate(num_states);

		double length = setup.getSolutionPath().length();
		cout << "Path length =" << length << endl;

		setup.getSolutionPath().printAsMatrix(std::cout);
		// Get all the transformations in the path

	}

	visualizePath(setup, obstacles, robot);
}

void findPathBetweenFibers(std::string obstacles, std::string robot,
		std::vector<fiber> allfibers) {

	std::vector<state> goal_states;

	cout << "Computing fiber graph" << endl;
	Graph fibgraph = fiberGraph(allfibers);

	cout << "Solving TSP" << endl;
	std::vector<unsigned int> path = solveTSP(fibgraph);

	cout << "Computing state goals" << endl;
	computeStateGoals(path, allfibers, goal_states);

	std::vector<state> uniqueGoals;
	cout << "Goal states = " << endl;
	for (auto i = (goal_states.begin()); i != (goal_states.end()); i++) {
		state s = (*i);
		if (i == goal_states.begin()) {
			uniqueGoals.push_back(s);
			printState(s);
			continue;
		}

		std::vector<state>::iterator prv = std::prev(i);
		state t = (*prv);
		//cout << RiemannianDistance(s, t) << endl;

		if (RiemannianDistance(s, t) > 1e-6) {
			uniqueGoals.push_back(s);
			printState(s);
		}
	}

	cout << "Starting motion planning" << endl;
	// read the stl files for the obstacle and robot
	findPath(obstacles, robot, uniqueGoals);

}
