/*
 * state.cpp
 *
 *  Created on: Jan 28, 2019
 *      Author: nelaturi
 */




#include "state.h"
#include <iostream>

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

void printState(state s){

	std::cout << s.x << "," << s.y << "," << s.z << "," << s.qx << ","
			<< s.qy << "," << s.qz << "," << s.qw << std::endl;
}
