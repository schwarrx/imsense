/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * se3metrics.h
 *
 *  Created on: Jan 23, 2019
 *      Author: saigopal nelaturi
 */

#ifndef SE3METRICS_H_
#define SE3METRICS_H_

#include <vector>
#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions> // for log

#include "state.h"

typedef std::vector<state> fiber;

Eigen::Matrix4d state2Matrix(state s);
double RiemannianDistance(state s1, state s2);
double fiberDistance(fiber f1, fiber f2);
double stateFiberDistance(state s, fiber f);

void printFiber(fiber f);
std::vector<state> closestStates(fiber f1, fiber f2);

#endif /* SE3METRICS_H_ */
