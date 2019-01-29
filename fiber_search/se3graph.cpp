/*
 * se3graph.cpp
 *
 *  Created on: Jan 24, 2019
 *      Author: nelaturi
 */

#include "se3graph.h"
#include <iostream>
#include <fstream>

#include <boost/graph/metric_tsp_approx.hpp>

using namespace std;
using namespace boost;

typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
typedef vector<graph_traits<Graph>::vertex_descriptor> vcontainer;

struct mvisitor: boost::default_dfs_visitor {
	template<typename Graph>
	void tree_edge(Edge e, const Graph& g) {
		std::cout << "traversing: " << e << " with weight "
				<< get(boost::edge_weight, g, e) << "\n";
	}
};

void PrintEdges(Graph g, const char* name) {

	cout << "Printing graph structure for " << name << endl;
	cout << "---------------------------------------" << endl;
	typedef graph_traits<Graph>::edge_iterator edge_iter;
	std::pair<edge_iter, edge_iter> edgePair;
	property_map<Graph, edge_weight_t>::type EdgeWeightMap = get(
			edge_weight_t(), g);

	for (edgePair = edges(g); edgePair.first != edgePair.second;
			++edgePair.first) {
		std::cout << *(edgePair.first) << "-->"
				<< EdgeWeightMap[*edgePair.first] << std::endl;
	}

	std::cout << std::endl;
}

Graph fiberGraph(std::vector<fiber> fibers) {

	// create a graph and add all fibers as nodes.
	Graph g(fibers.size()); // Create a graph with as many vertices as there are fibers

	// now populate the graph by creating edges between every pair of nodes
	for (auto i = fibers.begin(); i != fibers.end(); i++) {
		for (auto j = fibers.begin(); j != fibers.end(); j++) {
			unsigned int findex_i = std::distance(fibers.begin(), i);
			unsigned int findex_j = std::distance(fibers.begin(), j);

			if ((i < j)) {
				// weight the edge by fiber distance
				EdgeWeightProperty e1 = fiberDistance(*i, *j);
				add_edge(findex_i, findex_j, e1, g);
			}

		}
	}
	const char* name = "fiber graph ";
	PrintEdges(g, name);
	return g;
}

std::vector<unsigned int> solveTSP(Graph g) {

	// compute the traveling salesman problem on the fiber graph
	typedef property_map<Graph, edge_weight_t>::type WeightMap;
	WeightMap weight_map(get(edge_weight, g));

	vcontainer c;
	double len = 0.0;

	// Run the TSP approx, creating the visitor on the fly.
	metric_tsp_approx(g,
			make_tsp_tour_len_visitor(g, back_inserter(c), len, weight_map));

	std::vector<unsigned int> path;

	for (vcontainer::iterator itr = c.begin(); itr != c.end(); ++itr) {
		path.push_back(c[*itr]);
	}

	return path;
}

std::vector<state> computeStateGoals(vector<unsigned int> path,
		std::vector<fiber> fibers,  vector<state> &goals) {
	// Given a path of vertices (fiber sequence) to traverse, and a reference state
	// compute the sequence of states which need to be traversed
	// by the tool to avoid collisions (in a greedy way)

	std::vector<state> goal_states;
	double mindist = 1e10;

	// make pairs from the path
	std::vector<std::pair<int, int>> num_pairs;
	if (!path.empty()) {
		std::transform(std::begin(path), std::prev(std::end(path)),
				std::next(std::begin(path)), std::back_inserter(num_pairs),
				std::make_pair<decltype(path)::const_reference,
						decltype(path)::const_reference>);
	}
	// now iterate
	for (auto i = num_pairs.begin(); i != std::prev(num_pairs.end()); i++) {

		auto fiberpair = *i;
		cout << fiberpair.first << "--" << fiberpair.second << endl;

		std::vector<state> closest = closestStates(fibers[fiberpair.first],
				fibers[fiberpair.second]);

		goals.insert(goals.end(), closest.begin(), closest.end());
	}

	return goals;

}

std::vector<Edge> computeMST(Graph g) {
	// use Kruskal's algorithm to find the minimal spanning tree

	property_map<Graph, edge_weight_t>::type weight = get(edge_weight, g);
	std::vector<Edge> spanning_tree;

	kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));

	std::cout << "Print the edges in the MST:" << std::endl;
	for (std::vector<Edge>::iterator ei = spanning_tree.begin();
			ei != spanning_tree.end(); ++ei) {
		std::cout << source(*ei, g) << " <--> " << target(*ei, g)
				<< " with weight of " << weight[*ei] << std::endl;
	}

	// write an image of the graph as dot file
	// then do dot -Tps kruskal-eg.dot -o outfile.ps in the figs directory
	std::ofstream fout("figs/kruskal-eg.dot");
	fout << "graph A {\n" << " rankdir=LR\n" << " size=\"3,3\"\n"
			<< " ratio=\"filled\"\n" << " edge[style=\"bold\"]\n"
			<< " node[shape=\"circle\"]\n";
	graph_traits<Graph>::edge_iterator eiter, eiter_end;
	for (boost::tie(eiter, eiter_end) = edges(g); eiter != eiter_end; ++eiter) {
		fout << source(*eiter, g) << " -- " << target(*eiter, g);
		if (std::find(spanning_tree.begin(), spanning_tree.end(), *eiter)
				!= spanning_tree.end())
			fout << "[color=\"black\", label=\"" << get(edge_weight, g, *eiter)
					<< "\"];\n";
		else
			fout << "[color=\"gray\", label=\"" << get(edge_weight, g, *eiter)
					<< "\"];\n";
	}
	fout << "}\n";

	/*
	 * DFS MST
	 *
	 *
	 struct InSpanning {
	 std::set<Edge> edges;
	 bool operator()(Edge e) const {
	 return edges.count(e);
	 }
	 } spanning;
	 *
	 *
	 * kruskal_minimum_spanning_tree(g,
	 std::inserter(spanning.edges, spanning.edges.end()));

	 mvisitor vis;
	 filtered_graph<Graph, InSpanning, boost::keep_all> mst(g, spanning, { });
	 depth_first_search(mst, visitor(vis));*/

	return spanning_tree;

}
