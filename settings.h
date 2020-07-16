#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include <string>

using namespace std;

enum Algorithm {
	None,
	BreadthFirstSearch,
	DepthFirstSearch,
	Dijkstra,
	FastDijkstra,
	BellmanFord,
	Dijkstra2D,
};

struct Settings {
	Algorithm Algorithm = None;
	int VertexCount = 6;
	int MaxEdgeCount = 8;
	bool SelfLoop = true;
	bool BiDirectional = true;
	bool Verbose = false;
	int MinEdgeWeight = 0;
	int MaxEdgeWeight = 0;
	string FilePath = "./graph.json";
	string SourceVertex = "";
	string TargetVertex = "";
	bool LoadFromFile = false;
	bool SaveToFile = false;

	void parse (int argc, char **argv);
	void print ();
};

#endif
