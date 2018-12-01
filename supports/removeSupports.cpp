/*
 * removeSupports.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: nelaturi
 */

#include "removeSupports.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <iterator>
#include <iomanip>      // std::setw
#include "helper.h"

af::array getDilatedPart(af::array part, float kernelSize) {
	// dilate the part -- useful for support intersections
	int d = part.numdims();
	af::array dilatedPart;

	if (d == 2) {
		af::array mask = constant(1, kernelSize, kernelSize);
		dilatedPart = dilate(part, mask);
	} else {
		af::array mask = constant(1, kernelSize, kernelSize, kernelSize);
		dilatedPart = dilate3(part, mask);
	}
	return (dilatedPart);
}

af::array getSupportComponents(af::array supports) {
	//get connected components -- need to cast input to b8 (binary 8 bit)
	// connected components will identify each support individually and label them.
	af::array components = af::regions(supports.as(b8), AF_CONNECTIVITY_8);
	// TODO -- add connected components code in 3d
	return (components);
}

af::array getDislocationFeatures(af::array dilatedPart, af::array supports) {
	/*
	 * Identify all the features of contact between each support and
	 * the part and store them. These will be needed when computing
	 * the contact fibers.
	 */
	// intersection is pointwise multiplication
	// identify where the dilated part intersects the supports
	af::array componentDislocations = dilatedPart * supports;
	return (componentDislocations);

}

af::array getEpsilonContactSpace(af::array nearNet, af::array tool,
		angleAxis rotation, float epsilon) {
	/*
	 * get the contact space for an oriented tool. Doing this as
	 * a separate function to avoid memory issues.
	 */
	bool correlate = true; // need cross correlation
	return (sublevelComplement(
			convolveAF2(nearNet,
					rotate(tool, rotation.angle, true,
							AF_INTERP_BICUBIC_SPLINE), correlate), epsilon));
}

af::array getProjectedContactCSpace(af::array nearNet, af::array tool,
		std::vector<angleAxis> rotations, float epsilon) {
	/*
	 * Given a nearNet and a tool in d dimensions, get the
	 * d* (d+1)/2 dimensional configuration space and extract
	 * the contact configurations, by identifying the configurations
	 * where the overlap measure is less than epsilon. Furthermore
	 * the support removal algorithm only requires the projection of
	 * this contact space.
	 */

	af::array projectedContactCSpace = constant(0, nearNet.dims(), f32);
	int n = static_cast<int>(rotations.size()); //number of rotations

	int d = tool.numdims(); // problem dimension
	dim4 resultDim = nearNet.dims(); // convolution will not expand input size
	int batchsize = getBatchSize(d, nearNet.dims()[0], tool.dims()[0],
			resultDim[0]);

	af::timer::start();

	// see https://github.com/arrayfire/arrayfire/issues/1709
	for (int i = 0; i < n; i++) { // can we gfor this?
		// do cross correlation and return all voxels where the overlap
		// field value is less than a measure;
		// TODO -- add fancy code to template whether to use
		// convolveAF2 or AF3 depending on nearNet.numdims()
		projectedContactCSpace += getEpsilonContactSpace(nearNet, tool,
				rotations[i], epsilon);
		//printGPUMemory();

		if (n % batchsize == 0) {
			// this is required to avoid memory blowup, see github link above
			af::eval(projectedContactCSpace);
			// periodically do garbage collection .. lame.
			// AF has some bug with non-expanded convolution
			af::deviceGC();
		}
	}
	cout << "Done computing projected contact space in  " << af::timer::stop()
			<< " s" << endl;
	return (projectedContactCSpace);

}

void peels(std::vector<std::vector<int> > L, unsigned int nSupports) {
	if (L.empty()) {
		cout << "None of the supports are accessible" << endl;
	} else {
		if (L.size() != nSupports) {
			cout << "Some supports are not reachable" << endl;
		} else {
			cout << "All supports are removable" << endl;
		}
		cout << "The peeling sequence for reachable supports is listed below"
				<< endl;
		for (auto iter = L.begin(); iter != L.end(); iter++) {
			auto R = *iter;
			cout << "<";
			for (auto it = R.begin(); it != R.end(); it++) {
				cout << *it << ",";
			}
			cout << '\b' << ">" << endl;
		}
	}

}

double getRerror(af::array a, af::array b, int d) {
	// This is a way of checking whether two indicator functions are approx equal
	double error = count(af::where(a - b)).scalar<int>();
	double rerror = error / ((double) pow(a.dims()[0], d)); // relative error
	return rerror;
}

template<typename T>
std::vector<T> flatten(const std::vector<std::vector<T>>& v) {
	// from stack overflow
	std::size_t total_size = 0;
	for (const auto& sub : v)
		total_size += sub.size();
	std::vector<T> result;
	result.reserve(total_size);
	for (const auto& sub : v)
		result.insert(result.end(), sub.begin(), sub.end());
	return result;
}

std::vector<std::vector<int> > removeSupports(af::array nearNet, af::array tool,
		af::array part, af::array components, af::array dislocations,
		std::vector<angleAxis> rotations, float epsilon,
		std::vector<std::vector<int> > L, int nSupports, int count) {
	/*
	 * Recursive algorithm to remove supports
	 * L is the vector of maximally removable supports
	 */

	int d = nearNet.numdims(); // problem dimension
	double atol = 1e-4; // absolute tolerance for numerical error
	// base-case
	if (getRerror(nearNet, part, d) < atol) {
		peels(L, nSupports); // print
		return L; // all supports are removed
	}

	// Compute the projected contact space
	af::array piContactCSpace = getProjectedContactCSpace(nearNet, tool,
			rotations, epsilon);

	af::eval(piContactCSpace);
	// Now check if the trimmed projection contains some dislocation features.
	// To do this, check where the trimmed projection function intersects the
	// dislocation features.
	af::array accessibleDislocations = piContactCSpace * dislocations;
	af::array removableSupports = setUnique(
			components(af::where(accessibleDislocations))).as(f32);
	int n = removableSupports.dims()[0]; // number of removable supports

	if (n == 0) {
		peels(L, nSupports); // print
		return L;
	}

	// only remove supports unique to this iteration -- there may be crud leftover
	// from previous support removals, but as long as they are within atol, we assume
	// some milling finishing operations will take care of it.
	auto iL = (flatten(L)); // index list of all previously removed supports
	set<int> sL(iL.begin(), iL.end()); // make iL into a set and remove duplicates
	// sL is the index set of all supports removed prior to this iteration
	set<int> iR; // index set of candidate removable supports in this iteration
	for (int i = 0; i < n; i++) {
		iR.insert((int) removableSupports(i).scalar<float>());
	}
	set<int> U; // uniquely removable supports
	std::set_difference(iR.begin(), iR.end(), sL.begin(), sL.end(),
			std::inserter(U, U.end())); // compute iR - iL to get unique suports

	// each support often has multiple components, make sure they are all removed.
	std::vector<int> R; // all supports removable in this iteration of the recursion
	for (auto iter = U.begin(); iter != U.end(); iter++) {
		// removableSupports(i) is an array and needs to be converted to a float
		int supportNum = *iter;
		af::array singleSupport = indicator(components == supportNum);
		// find the dislocation for this support
		af::array dislocation = singleSupport * dislocations;
		// find the accessible dislocation from the C-obstacle
		af::array accessibleDislocation = singleSupport
				* accessibleDislocations;

		if (getRerror(dislocation, accessibleDislocation, d) < atol) {
			// excellent, the entire support is removable
			R.push_back(supportNum);
			nearNet -= singleSupport; // subtract this support from the near-net shape
			af::eval(nearNet);
		} else {
			continue; // not a removable support
		}
	}
	count += 1;

	if (R.empty()) {
		peels(L, nSupports); // print
		return L;
	} else {
		L.push_back(R);

//		if (af::isImageIOAvailable()) {
//			stringstream filename;
//			filename <<  "nearNet" << std::setw(4) << std::setfill<char>('0') << count
//					<< ".png" << endl;
//			string fn = filename.str();
//			const char* file = fn.c_str();
//			af::saveImage(file, nearNet);
//		}

		visualize2D(nearNet);
		// recurse
		removeSupports(nearNet, tool, part, components, dislocations, rotations,
				epsilon, L, nSupports, count);
	}

	// avoid C++ warning/error -- control reaches end of non-void function [-Wreturn-type]
	return L; // code should never get to this
}

void runSupportRemoval(af::array nearNet, af::array tool, af::array part,
		float epsilon) {
	/*
	 * Run the support removal algorithm from start to end,
	 * including input checking and pre-processing
	 */

	// force indicator functions for the inputs
	nearNet = indicator(nearNet);
	tool = indicator(tool);
	part = indicator(part);

	checkInputs(nearNet, tool, part);

	int problemDimension = nearNet.numdims();
	std::vector<angleAxis> sampledRotations = getRotations(problemDimension);
	std::vector<std::vector<int> > maximallyRemovableSupports; // the output

	af::array supports = (nearNet - part); // the collection of all support structures

	float dilationKernelSize = 5; // how much to thicken the part to find the dislocation features
	af::array dislocations = getDislocationFeatures(
			getDilatedPart(part, dilationKernelSize), supports); // where the supports intersect the part

	// labeling all the dislocations by connected components
	af::array components = getSupportComponents(supports);
	// run the recursive support removal algo
	int nSupports = max<int>(components);
	int count = 0; // 0th recursion -- need numbering to save files ..
	cout << "Number of supports to be removed =" << nSupports << endl;
	removeSupports(nearNet, tool, part, components, dislocations,
			sampledRotations, epsilon, maximallyRemovableSupports, nSupports,
			count);

}

