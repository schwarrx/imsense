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
#include "state.h"

using namespace std;

int main(int argc, char *argv[]) {

	if ((argc != 5)) {
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

			// Ensure that the orientation entries are unit quaternions
			double qx, qy, qz, qw = 0;
			ori >> qx >> qy >> qz >> qw;
			Eigen::Quaterniond q;
			q.x() = qx;
			q.y() = qy;
			q.z() = qz;
			q.w() = qw;
			q.normalize();
			s.qx = q.x();
			s.qy = q.y();
			s.qz = q.z();
			s.qw = q.w();

			f.push_back(s);
		}
		allfibers.push_back(f);
	}

	std::string obstacle(argv[1]);
	std::string robot(argv[2]);
	std::string actual_obs(argv[4]);


	findPathBetweenFibers(obstacle, robot, allfibers, actual_obs);

	return 0;

}
