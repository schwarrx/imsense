/*
 *
 * Copyright (c) 2019, Palo Alto Research Center, Inc.
 *
 * main.cpp
 *
 * Created on: Jan 15, 2019
 *     Author: saigopal nelaturi
 */

#include "findpath.h"

int main(int argc, char *argv[]) {

	if ((argc != 17)) {
		cout << "Number of arguments = " << argc << endl;
		cout << "usage = " << endl;
		cout
				<< "./findPath obstacle robot start(x,y,z,ax,ay,ax,ang) end(x,y,z,ax,ay,az,ang) \n"
				<< endl;
		exit(1);
	}

	std::string obstacle(argv[1]);
	std::string robot(argv[2]);

	state initial;
	initial.x = double(atof(argv[3]));
	initial.y = double(atof(argv[4]));
	initial.z = double(atof(argv[5]));
	initial.axis_x = double(atof(argv[6]));
	initial.axis_y = double(atof(argv[7]));
	initial.axis_z = double(atof(argv[8]));
	initial.angle = double(atof(argv[9]));

	state final;
	final.x = double(atof(argv[10]));
	final.y = double(atof(argv[11]));
	final.z = double(atof(argv[12]));
	final.axis_x = double(atof(argv[13]));
	final.axis_y = double(atof(argv[14]));
	final.axis_z = double(atof(argv[15]));
	final.angle = double(atof(argv[16]));

	findPath(obstacle, robot, initial, final);

	return 0;

}