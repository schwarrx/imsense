/*******************************************************
 * Author - S Nelaturi
 * 
 ********************************************************/

// Note : use cmake -DArrayFire_DIR=/home/nelaturi/arrayfire ..

#include <stdio.h>
#include <iostream>

#include "track.h"

int main(int argc, char** argv)
{
	CommandLineParser parser(argc, argv, "{help h||}{@input||}");
	if (parser.has("help"))
	{
		std::cout << "usage: ./a2pcv-cuda videofile " << std::endl;
		return 0;
	}
	string input = parser.get<std::string>("@input");
	trackParticles(input);


	return 0;
}




