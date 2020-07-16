/*
 * graph.h
 *
 *  Created on: 3 џэт. 2020 у.
 *      Author: Serkin
 */

#ifndef INC_GRAPH_H_
#define INC_GRAPH_H_

#include <string>
#include <vector>
#include <map>
#include "settings.h"

using namespace std;

struct Edge;

typedef vector<Edge*> EdgeList;

struct Vertex {
	string Name;
	EdgeList *OutcomingEdges = nullptr;
	EdgeList *IncomingEdges = nullptr;
	void* Context = nullptr;
	Vertex(string _Name);
	virtual ~Vertex();
	//makes copy of vertex. Edges and context are not cloned
	virtual Vertex* edgelessClone();
};

typedef Vertex* PVertex;

struct Edge {
	Vertex *FromVertex;
	Vertex *ToVertex;
	double Weight;
	Edge(Vertex* _FromVertex, Vertex* _ToVertex, double _Weight):
		FromVertex(_FromVertex), ToVertex(_ToVertex), Weight(_Weight) {};
};
typedef Edge* PEdge;
typedef map<string, Vertex*> Graph;

//Add edge from one given vertex to another given vertex
Edge* addEdge (Vertex *from, Vertex *to, const double weight, Graph &graph, const Settings& settings);

//Add vertex to a graph
bool addVertex (Vertex *vertex, Graph &graph, const Settings& settings);

//Create vertex and add it to a graph
Vertex* addVertex (const string &name, Graph &graph, const Settings& settings);

//Remove vertex from the graph
//All context information should be previously deleted by calling clearContext()
void removeVertex (Vertex **vertex, Graph &graph);

//find vertex with the given name in the graph
Vertex* findVertex(const string &name, const Graph &graph);

//Makes a clone of the graph
//vertices and edge are cloned but not a context
//graph - graph to make clone from
//returns new graph which is a clone of given
//Graph is allocated with new expression.
Graph& cloneGraph(const Graph &graph);

//Deletes all vertices from graph
//All context information should be previously deleted by calling clearContext()
void clearGraph(Graph &graph);

#endif /* INC_GRAPH_H_ */
