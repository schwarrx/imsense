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
#include "se3graph.h"
#include "findpath.h"

using namespace std;

int main(int argc, char *argv[]) {

	if ((argc != 4)) {
		cout << "Number of arguments = " << argc << endl;
		cout << "usage = " << endl;
		cout << "./fiberSearch obstacle robot fibers \n" << endl;
		exit(1);
	}

	// read path and transform states
	std::ifstream infile(argv[3]);
	std::string line;
	cout << "Reading fibers" << endl;

	// store a vector of fibers
	std::vector<fiber> allfibers;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);
		std::istringstream ori(line);
		int columns = 0;

		double x, y, z = 0;
		iss >> x >> y >> z;

		do {
			std::string sub;
			iss >> sub;
			if (sub.length())
				++columns;
		} while (iss);

		ori >> x >> y >> z;

		int norientations = (columns) / 4;
		fiber f;
		for (int i = 0; i < norientations; i++) {
			state s;
			s.x = x;
			s.y = y;
			s.z = z;
			double qx, qy, qz, qw = 0;
			ori >> qx >> qy >> qz >> qw;
			s.qx = qx;
			s.qy = qy;
			s.qz = qz;
			s.qw = qw;

			f.push_back(s);
		}
		allfibers.push_back(f);
	}

	state ref;
	ref.x = 20.0;
	ref.y = -30.0;
	ref.z = 20.0;
	ref.qx = 0.0;
	ref.qy = 0.0;
	ref.qz = 0.0;
	ref.qw = 1.0;

	vector<state> goal_states;
	goal_states.push_back(ref);

	cout << "Computing fiber graph" << endl;
	Graph fibgraph = fiberGraph(allfibers);
	cout << "Solving TSP" << endl;
	vector<unsigned int> path = solveTSP(fibgraph);
	cout << "Computing state goals" << endl;
	computeStateGoals(path, allfibers, ref, goal_states);
	//goal_states.push_back(ref);

	cout << "Goal states = " << endl;
	for (auto i = goal_states.begin(); i != goal_states.end(); i++) {
		state s = (*i);
		// print state
		std::cout << s.x << "," << s.y << "," << s.z << "," << s.qx << ","
				<< s.qy << "," << s.qz << "," << s.qw << std::endl;

	}

	cout << "Starting motion planning" << endl;
	// read the stl files for the obstacle and robot
	std::string obstacle(argv[1]);
	std::string robot(argv[2]);
	findPath(obstacle, robot, goal_states);

	return 0;

}
