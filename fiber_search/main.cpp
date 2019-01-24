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

using namespace std;

int main(int argc, char *argv[]) {

	// read path and transform states
	std::ifstream infile(argv[1]);
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

	Graph fibgraph = fiberGraph(allfibers);
	std::vector<Edge> mst = computeMST(fibgraph);
	fiberDijkstra(fibgraph);

	return 0;

}
