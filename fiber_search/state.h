/*
 * state.h
 *
 *  Created on: Jan 24, 2019
 *      Author: nelaturi
 */

#ifndef STATE_H_
#define STATE_H_

#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions> // for log



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


typedef std::vector<state> fiber;

Eigen::Matrix4d state2Matrix(state s);
void printState(state s);

struct compareStates {
	bool operator()(const state &lhs, const state &rhs) {
		double norm1 = state2Matrix(lhs).log().norm();
		double norm2 = state2Matrix(rhs).log().norm();
		return (norm1 < norm2);
	}
};

#endif /* STATE_H_ */
