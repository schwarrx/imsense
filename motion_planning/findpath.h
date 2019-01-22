/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * findpath.h
 *
 * Created on: Jan 15, 2019
 *     Author: saigopal nelaturi
 */

#ifndef PATH_H
#define PATH_H

#include <iostream>
#include <vector>

struct state {
	// describes a configuration in SE(3)
	// translation components
	double x;
	double y;
	double z;
	// rotation components as quaternions
	double qx;
	double qy;
	double qz;
	double qw;
};

// solve the motion planning problem
void findPath(std::string obstacles, std::string robot, std::vector<state> goal_states);

#endif
