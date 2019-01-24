/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * se3metrics.cpp
 *
 *  Created on: Jan 23, 2019
 *      Author: saigopal nelaturi
 */

#include "se3metrics.h"
#include <iostream>

using namespace std;
void printFiber(fiber f) {
	// print all the states in a fiber
	for (auto it = f.begin(); it != f.end(); it++) {
		// read state
		state s = (*it);
		// print state
		std::cout << s.x << "," << s.y << "," << s.z << "," << s.qx << ","
				<< s.qy << "," << s.qz << "," << s.qw << std::endl;
	}

}

Eigen::Matrix4d state2Matrix(state s) {
	// take a state and return the equivalent 4x4 homogeneous transformation matrix
	Eigen::Matrix4d mat = Eigen::Matrix4d::Zero();
	// rotation part
	Eigen::Quaterniond q;
	q.x() = s.qx;
	q.y() = s.qy;
	q.z() = s.qz;
	q.w() = s.qw;
	q.normalize();
	Eigen::Matrix3d rot = q.toRotationMatrix();
	mat.block<3, 3>(0, 0) = rot;
	// translation part
	Eigen::Vector3d t;
	t[0] = s.x;
	t[1] = s.y;
	t[2] = s.z;
	mat.block<3, 1>(0, 3) = t;
	// homogeneous
	mat(3, 3) = 1;
	return mat;

}

double RiemannianDistance(state s1, state s2) {
	// compute the Riemannian distance between two states in SE(3)
	Eigen::Matrix4d m1 = state2Matrix(s1);
	Eigen::Matrix4d m2 = state2Matrix(s2);
	// Riemannian distance
	double dist = (m1.inverse() * m2).log().norm();
	return dist;
}

double stateFiberDistance(state s, fiber f) {
	// distance from a state to a fiber
	double mindist = 1e10;
	for (auto i = f.begin(); i != f.end(); i++) {
		state s1 = (*i);
		double dist = RiemannianDistance(s, s1);
		if (dist < mindist) {
			mindist = dist;
		}
	}
	return mindist;

}

double fiberDistance(fiber f1, fiber f2) {
	// compute the shortest distance between two fibers
	double mindist = 1e10;
	for (auto i = f1.begin(); i != f1.end(); i++) {
		for (auto j = f2.begin(); j != f2.end(); j++) {

			state s1 = (*i);
			state s2 = (*j);

			double dist = RiemannianDistance(s1, s2);
			if (dist < mindist) {
				mindist = dist;

			}
		}
	}
	return mindist;
}

std::vector<state> closestStates(fiber f1, fiber f2) {
	// compute the closest states between two fibers
	state sf1;
	state sf2;
	double mindist = 1e10;
	for (auto i = f1.begin(); i != f1.end(); i++) {
		for (auto j = f2.begin(); j != f2.end(); j++) {

			state s1 = (*i);
			state s2 = (*j);

			double dist = RiemannianDistance(s1, s2);
			if (dist < mindist) {
				mindist = dist;
				sf1 = s1;
				sf2 = s2;

			}
		}
	}
	std::vector<state> closest;
	closest.push_back(sf1);
	closest.push_back(sf2);
	return closest;
}
