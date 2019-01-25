/*
 * state.h
 *
 *  Created on: Jan 24, 2019
 *      Author: nelaturi
 */

#ifndef STATE_H_
#define STATE_H_


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


#endif /* STATE_H_ */
