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
#include <ompl/geometric/planners/kpiece/LBKPIECE1.h>
#include <iterator>
#include <vector>

using namespace ompl;
namespace ob = ompl::base;
namespace og = ompl::geometric;


void findPath(std::string obstacles, std::string robot, state start_state,
		state goal_state) {

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
			std::make_shared < og::RRTConnect > (setup.getSpaceInformation()));

	// define the state space as SE(3)
	ob::StateSpacePtr space(new ob::SE3StateSpace);

	// define compound start and goal states
	ob::ScopedState<base::SE3StateSpace> start(setup.getSpaceInformation());
	ob::ScopedState<base::SE3StateSpace> goal(setup.getSpaceInformation());

	start->setXYZ(start_state.x, start_state.y, start_state.z);
	start->rotation().x = start_state.qx;
	start->rotation().y = start_state.qy;
	start->rotation().z = start_state.qz;
	start->rotation().w = start_state.qw;

	goal->setXYZ(goal_state.x, goal_state.y, goal_state.z);
	goal->rotation().x = goal_state.qx;
	goal->rotation().y = goal_state.qy;
	goal->rotation().z = goal_state.qz;
	goal->rotation().w = goal_state.qw;

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

		visualizePath(setup, obstacles, robot);

	}

}

void findPathBetweenFibers(std::string obstacles, std::string robot,
		std::vector<fiber> allfibers) {

	std::vector<state> goal_states;

	cout << "Computing fiber graph" << endl;
	Graph fibgraph = fiberGraph(allfibers);

	cout << "Solving TSP" << endl;
	std::vector<unsigned int> path = solveTSP(fibgraph);

	cout << "Computing goal states for fiber path ";
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

	// make pairs from the path
		std::vector<std::pair<state, state>> state_pairs;
		if (!uniqueGoals.empty()) {
			std::transform(std::begin(uniqueGoals), std::prev(std::end(uniqueGoals)),
					std::next(std::begin(uniqueGoals)), std::back_inserter(state_pairs),
					std::make_pair<decltype(uniqueGoals)::const_reference,
							decltype(uniqueGoals)::const_reference>);
		}

		//const char* name = "file.txt";
		//std::ofstream out;
		//out.open(name, std::ios_base::app);

		for (auto i = state_pairs.begin(); i!= state_pairs.end(); i++){
			findPath(obstacles, robot, (*i).first, (*i).second);
		}



}
