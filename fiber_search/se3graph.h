/*
 * se3graph.h
 *
 *  Created on: Jan 24, 2019
 *      Author: nelaturi
 */

#ifndef SE3GRAPH_H_
#define SE3GRAPH_H_

#include "se3metrics.h"

// STL
#include <iostream>                  // for std::cout

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_graph.hpp>// A subclass to provide reasonable arguments to adjacency_list for a typical undirected graph
#include <boost/graph/graph_traits.hpp>

#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>


typedef boost::property<boost::edge_weight_t, double> EdgeWeightProperty;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
		boost::no_property, EdgeWeightProperty> Graph;

using GraphType = boost::adjacency_list<boost::setS, // OutEdgeContainer
boost::vecS,// VertexContainer
boost::undirectedS, boost::no_property, EdgeWeightProperty>;

typedef boost::graph_traits<Graph>::edge_descriptor Edge;


Graph fiberGraph(std::vector<fiber> &fibers);
std::vector<Edge> computeMST(Graph g);
std::vector<unsigned int>  solveTSP(Graph g);

std::vector<state> computeStateGoals(std::vector<unsigned int> path,
		std::vector<fiber> fibers,  std::vector<state> &goals);

#endif /* SE3GRAPH_H_ */
