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
#include <omplapp/geometry/detail/FCLStateValidityChecker.h>
#include <ompl/geometric/planners/kpiece/LBKPIECE1.h>
#include <ompl/geometric/planners/prm/SPARS.h>
#include <ompl/base/objectives/PathLengthOptimizationObjective.h>
#include <ompl/base/objectives/MaximizeMinClearanceObjective.h>
#include <ompl/base/objectives/StateCostIntegralObjective.h>
#include <iterator>
#include <vector>

// The supported optimal planners, in alphabetical order
#include <ompl/geometric/planners/bitstar/BITstar.h>
#include <ompl/geometric/planners/cforest/CForest.h>
#include <ompl/geometric/planners/fmt/FMT.h>
#include <ompl/geometric/planners/fmt/BFMT.h>
#include <ompl/geometric/planners/prm/PRMstar.h>
#include <ompl/geometric/planners/rrt/InformedRRTstar.h>
#include <ompl/geometric/planners/rrt/RRTstar.h>
#include <ompl/geometric/planners/rrt/SORRTstar.h>

 #include <memory>

using namespace ompl;
namespace ob = ompl::base;
namespace og = ompl::geometric;

// An enum of the supported optimization objectives, alphabetical order
enum planningObjective {
	OBJECTIVE_PATHCLEARANCE, OBJECTIVE_PATHLENGTH
};

// An enum of supported optimal planners, alphabetical order
enum optimalPlanner {
	PLANNER_BFMTSTAR,
	PLANNER_BITSTAR,
	PLANNER_CFOREST,
	PLANNER_FMTSTAR,
	PLANNER_INF_RRTSTAR,
	PLANNER_PRMSTAR,
	PLANNER_RRTSTAR,
	PLANNER_SORRTSTAR,
};

class ClearanceObjective: public ob::StateCostIntegralObjective {
public:
	ClearanceObjective(const ob::SpaceInformationPtr& si) :
			ob::StateCostIntegralObjective(si, true) {
	}
	// Our requirement is to maximize path clearance from obstacles,
	// but we want to represent the objective as a path cost
	// minimization. Therefore, we set each state's cost to be the
	// reciprocal of its clearance, so that as state clearance
	// increases, the state cost decreases.
	ob::Cost stateCost(const ob::State* s) const override
	{
		return ob::Cost(
				1
						/ (si_->getStateValidityChecker()->clearance(s)
								+ std::numeric_limits<double>::min()));
	}
};

ob::OptimizationObjectivePtr getClearanceObjective(
		const ob::SpaceInformationPtr& si) {
	return std::make_shared<ClearanceObjective>(si);
}

ob::OptimizationObjectivePtr getPathLengthObjective(
		const ob::SpaceInformationPtr& si) {
	return std::make_shared<ob::PathLengthOptimizationObjective>(si);
}

/*class ValidityChecker: public ob::StateValidityChecker {
public:
	ValidityChecker(const ob::SpaceInformationPtr& si) :
			ob::StateValidityChecker(si) {
	}
	// Returns whether the given state's position overlaps the
	// circular obstacle
	bool isValid(const ob::State* mystate) const {
		return this->clearance(mystate) > 0.0;
	}
	// Returns the distance from the given state's position to the
	// boundary of the circular obstacle.
	double clearance(const ob::State* mystate) const {
		 const ob::SE3StateSpace::StateType* state_g =
		 mystate->as<ob::SE3StateSpace::StateType>();
		 state g;
		 g.x = state_g->getX();
		 g.y = state_g->getY();
		 g.z = state_g->getZ();
		 g.qx = state_g->rotation().x;
		 g.qy = state_g->rotation().y;
		 g.qz = state_g->rotation().z;
		 g.qw = state_g->rotation().w;


		// set clearance of 0.25
		return 0.025;
	}
};*/


ob::OptimizationObjectivePtr allocateObjective(
		const ob::SpaceInformationPtr& si, planningObjective objectiveType) {
	switch (objectiveType) {
	case OBJECTIVE_PATHCLEARANCE:
		return getClearanceObjective(si);
		break;
	case OBJECTIVE_PATHLENGTH:
		return getPathLengthObjective(si);
		break;
	default:
		OMPL_ERROR(
				"Optimization-objective enum is not implemented in allocation function.");
		return ob::OptimizationObjectivePtr();
		break;
	}
}

ob::PlannerPtr allocatePlanner(ob::SpaceInformationPtr si,
		optimalPlanner plannerType) {
	switch (plannerType) {
	case PLANNER_BFMTSTAR: {
		return std::make_shared<og::BFMT>(si);
		break;
	}
	case PLANNER_BITSTAR: {
		return std::make_shared<og::BITstar>(si);
		break;
	}
	case PLANNER_CFOREST: {
		return std::make_shared<og::CForest>(si);
		break;
	}
	case PLANNER_FMTSTAR: {
		return std::make_shared<og::FMT>(si);
		break;
	}
	case PLANNER_INF_RRTSTAR: {
		return std::make_shared<og::InformedRRTstar>(si);
		break;
	}
	case PLANNER_PRMSTAR: {
		return std::make_shared<og::PRMstar>(si);
		break;
	}
	case PLANNER_RRTSTAR: {
		return std::make_shared<og::RRTstar>(si);
		break;
	}
	case PLANNER_SORRTSTAR: {
		return std::make_shared<og::SORRTstar>(si);
		break;
	}
	default: {
		OMPL_ERROR(
				"Planner-type enum is not implemented in allocation function.");
		return ob::PlannerPtr(); // Address compiler warning re: no return value.
		break;
	}
	}
}

void findPath(std::string obstacles, std::string robot, state start_state,
		state goal_state, std::string actual_obs, std::string actual_part) {

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
	setup.setPlanner(std::make_shared<og::RRTstar>(setup.getSpaceInformation()));

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

	// print setup info
	//setup.setup();
	//setup.print();

	// set start and goal states
	setup.setStartAndGoalStates(start, goal);

	// Create the optimization objective specified by our command-line argument.
	// This helper function is simply a switch statement.
	setup.setOptimizationObjective(
			allocateObjective(setup.getSpaceInformation(),
					OBJECTIVE_PATHCLEARANCE));


	double runTime = 20;
	// try to solve the motion planning problem
	if (setup.solve(runTime)) {
		// simplify & print the solution
		setup.simplifySolution();
		int num_states = 10;
		setup.getSolutionPath().interpolate(num_states);

		double length = setup.getSolutionPath().length();
		cout << "Path length =" << length << endl;

		setup.getSolutionPath().printAsMatrix(std::cout);
		// Get all the transformations in the path

		visualizePath(setup, actual_obs, robot, actual_part);

	}

}

void findPathBetweenFibers(std::string obstacles, std::string robot,
		std::vector<fiber> allfibers, std::string actual_obs,
		std::string actual_part) {

	std::vector<state> goal_states;

	cout << "Computing fiber graph" << endl;
	Graph fibgraph = fiberGraph(allfibers);

	cout << "Solving TSP" << endl;
	std::vector<unsigned int> path = solveTSP(fibgraph);

	cout << "Computing the MST" << endl;
	computeMST(fibgraph);

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
		std::transform(std::begin(uniqueGoals),
				std::prev(std::end(uniqueGoals)),
				std::next(std::begin(uniqueGoals)),
				std::back_inserter(state_pairs),
				std::make_pair<decltype(uniqueGoals)::const_reference,
						decltype(uniqueGoals)::const_reference>);
	}

	//const char* name = "file.txt";
	//std::ofstream out;
	//out.open(name, std::ios_base::app);

	for (auto i = state_pairs.begin(); i != state_pairs.end(); i++) {
		findPath(obstacles, robot, (*i).first, (*i).second, actual_obs,
				actual_part);
	}

}
