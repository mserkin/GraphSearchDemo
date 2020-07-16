/*
 * settings.cpp
 *
 *  Created on: 7 нояб. 2019 г.
 *      Author: Serkin
 */

#include <string>
#include <unistd.h>
#include <map>
#include <algorithm>
#include <cctype>
#include <string>
#include <getopt.h>
#include <iostream>
#include "settings.h"

map <Algorithm, string> AlgorithmNames = {
		{None, ""},
		{BreadthFirstSearch, "Breadth-first search"},
		{DepthFirstSearch, "Depth-first search"},
		{Dijkstra, "Dijkstra"},
		{FastDijkstra, "Fast Dijkstra"},
		{BellmanFord, "Bellman-Ford"},
		{Dijkstra, "Dijkstra2D"}
};

map <string, Algorithm> AlgorithmArgs = {
		{"bfs", BreadthFirstSearch},
		{"dfs", DepthFirstSearch},
		{"dijkstra", Dijkstra},
		{"dijkstra2d", Dijkstra2D},
		{"fast-dijkstra", FastDijkstra},
		{"bellman-ford", BellmanFord}
};

static const option longOpts[] = {
    { "vertex-count", required_argument, NULL, 'V' },
    { "edge-count", required_argument, NULL, 'E' },
    { "self-loops", no_argument, NULL, 'L' },
    { "bi-dir", no_argument, NULL, 'B' },
	{ "min-weight", required_argument, NULL, 'w' },
	{ "max-weight", required_argument, NULL, 'W' },
    { "file", required_argument, NULL, 'F' },
	{ "save", no_argument, NULL, 's' },
	{ "load", no_argument, NULL, 'l' },
	{ "algorithm", required_argument, NULL, 'a' },
	{ "source-vertex", required_argument, NULL, 'S' },
	{ "target-vertex", required_argument, NULL, 'T' },
	{ "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

static const char *optString = "a:E:V:vw:W:LBF:S:T:hls?";

void displayUsage () {
	cout << " GraphCreator [OPTIONS]\nOptions are:\n";
	cout << "\t-F, --file\t\t\tPath to a file\n";
	cout << "\t-l, --load\t\t\tLoad graph to a file\n";
	cout << "\t-s, --save\t\t\tSave graph from file\n";
	cout << "\t-V, --vertex-count\t\tNumber of vertices in the graph\n";
	cout << "\t-E, --edge-count\t\tMaximum number of edges in the graph\n";
	cout << "\t-L, --self-loops\t\tProhibit edges that join a vertex to itself\n";
	cout << "\t-B, --bi-dir\t\t\tProhibit bi-directional edges\n";
	cout << "\t-w, --min-weight\t\tMinimum weight \n";
	cout << "\t-W, --max-weight\t\tMaximum weight \n";
	cout << "\t-a, --algorithm\t\t\tAlgorithm to use: bfs, dfs, dijkstra, dijkstra2d, fast-dijkstra, bellman-ford\n";
	cout << "\t-S, --source-vertex\t\tSource vertex for single-source search\n";
	cout << "\t-T, --target-vertex\t\tTarget vertex to find path to\n";
	cout << "\t-v, --verbose\t\t\tPrint additional information\n";
	cout << "\t-h, --help\t\t\tShow this help information\n";
	exit(0);
}

void Settings::print () {
	if (!this->Verbose) return;
	cout << ">>> Settings:\n";
	cout << "\tVertex count = " << this->VertexCount << "\n";
	cout << "\tMaximum edge count = " << this->MaxEdgeCount << "\n";
	if (this->SelfLoop) {
		cout << "\tself-loops are possible\n";
	}
	if (this->BiDirectional) {
		cout << "\tGraph can be bi-directional\n";
	}
	cout << "\tMinimum edge weight = " << this->MinEdgeWeight<< "\n";
	cout << "\tMaximum edge weight = " << this->MaxEdgeWeight<< "\n";
	cout << "\tFile path = " << this->FilePath << "\n";
	if (this->LoadFromFile) {
		cout << "\tLoad graph from the file."<< "\n";
	}
	else {
		cout << "\tGenerate graph."<< "\n";
	}
	if (this->SaveToFile) {
		cout << "\tSave graph to the file."<< "\n";
	}

	if (this->Algorithm != None) {
		cout << "\tApply algorithm: " << AlgorithmNames[this->Algorithm] << "\n";

		if (this->SourceVertex != "") {
			cout << "\tSource vertex: " << this->SourceVertex << "\n";
		}
		if (this->TargetVertex != "") {
			cout << "\tTarget vertex: " << this->TargetVertex << "\n";
		}
	}
}

void Settings::parse (int argc, char **argv) {
	int longIndex, opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	while( opt != -1 ) {
		switch( opt ) {
			case 'V':
				this->VertexCount = atoi(optarg);
				break;

			case 'E':
				this->MaxEdgeCount = atoi(optarg);
				break;

			case 'L':
				this->SelfLoop = false;
				break;

			case 'B':
				this->BiDirectional = false;
				break;

			case 'v':
				this->Verbose = true;
				break;

			case 'F':
				this->FilePath = optarg;
				break;

			case 'l':
				this->LoadFromFile = true;
				break;

			case 's':
				this->SaveToFile = true;
				break;

			case 'a': {
				string s(optarg);
				transform(s.begin(), s.end(), s.begin(),
				    [](unsigned char c){ return std::tolower(c); });
				if (AlgorithmArgs.find(s) == AlgorithmArgs.end()) {
					cerr << "Unknown algorithm: " << s << endl;
					exit(-20);
				}
				this->Algorithm = AlgorithmArgs[s];
				break;
			}
			case 'S':
				this->SourceVertex = optarg;
				break;

			case 'T':
				this->TargetVertex = optarg;
				break;

			case 'w':
				this->MinEdgeWeight = atoi(optarg);
				break;

			case 'W':
				this->MaxEdgeWeight = atoi(optarg);
				break;

			case 'h':   /* намеренный проход в следующий case-блок */
			case '?':
				displayUsage();
				break;

			default:
				/* сюда на самом деле попасть невозможно. */
				break;
		}

		opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	}
}
