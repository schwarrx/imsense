/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * main.cpp
 *
 * Created on: Jan 15, 2019
 *     Author: saigopal nelaturi
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include "findpath.h"

using namespace std;

int main(int argc, char *argv[]) {

	if ((argc != 4)) {
		cout << "Number of arguments = " << argc << endl;
		cout << "usage = " << endl;
		cout << "./findPath obstacle robot goal_states \n" << endl;
		exit(1);
	}

	// read the stl files for the obstacle and robot
	std::string obstacle(argv[1]);
	std::string robot(argv[2]);

	// read a list of all the goal states ---
	// we assume that the user will provide a file
	// that contains a sequence of states,
	// i.e. the order in which the goal states are specified
	// matters. Each new line in the file represents a goal
	// state for the robot which needs to be reached
	// starting from the state in the previous line. Thus the
	// first line in the file has to represent the starting
	// state.

	// read path and transform states
	std::ifstream infile(argv[3]);
	std::string line;
	cout << "Reading goal states" << endl;
	// store a vector of all goal states
	std::vector<state> goal_states;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);
		double x, y, z, qx, qy, qz, qw;
		if (!(iss >> x >> y >> z >> qx >> qy >> qz >> qw)) {
			cout << "File error -- check that the goal states are properly specified" << endl;
			break;
		} // error

		state goal;
		goal.x = x;
		goal.y = y;
		goal.z = z;
		goal.qx = qx;
		goal.qy = qy;
		goal.qz = qz;
		goal.qw = qw;
		// push back this state to the vector
		goal_states.push_back(goal);
		cout << "." ;

	}
	cout << endl;
	findPath(obstacle, robot, goal_states);

	return 0;

}
