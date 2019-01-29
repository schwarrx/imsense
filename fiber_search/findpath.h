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

#include "state.h"

// solve the motion planning problem

void findPathBetweenFibers(std::string obstacles, std::string robot,
		std::vector<fiber> allfibers);

void findPath(std::string obstacles, std::string robot,
		std::vector<state> goal_states);

#endif
