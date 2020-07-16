/*
 * creator.cpp
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#include <iostream>
#include "graph.h"
#include "settings.h"
#include "error.h"

Vertex* chooseToVertex(Vertex *vertex_from, const Settings &settings, Graph &graph,
		int &error_count, int &error) {

	int vertex_ind = rand() % settings.VertexCount;
	auto it = graph.begin();
	advance(it, vertex_ind);
	Vertex *vertex_to = it->second;
	if (!settings.SelfLoop && vertex_to == vertex_from) {
		error_count++;
		error = FATAL_ERROR_SELF_LOOPS_ARE_PROHIBITED;
		return nullptr;
	}
	if (!settings.BiDirectional) {
		for (auto itb = vertex_to->OutcomingEdges->begin(); itb != vertex_to->OutcomingEdges->end(); itb++) {
			if ((*itb)->ToVertex == vertex_from) {
				error_count++;
				error = FATAL_ERROR_BIDIRECTION_EDGES_ARE_PROHIBITED;
				return nullptr;
			}
		}
	}
	for (auto ite = vertex_from->OutcomingEdges->begin(); ite != vertex_from->OutcomingEdges->end(); ite++) {
		if ((*ite)->ToVertex == vertex_to) {
			error_count++;
			error = FATAL_ERROR_EDGE_ALREADY_EXISTS;
			return nullptr;
		}
	}
	return vertex_to;
}


void createGraph(Graph& graph, const Settings& settings) {
	int error_count = 0;
	int last_error = 0;
	int total_edge_num = settings.MaxEdgeCount;
	for (int i = 0; i < settings.VertexCount; i++) {
		char *name = new char[4]; name[0] = 'V';
		sprintf(name+1, "%X", i);
		//itoa(i, name+1, 36);
		Vertex *vertex = addVertex(name, graph, settings);
		if (vertex == nullptr) {
			cerr << "Cannot add vertex to graph.";
			exit(FATAL_ERROR_UNKNOWN_FILE_TYPE);
		}
	}

	while (total_edge_num > 0 && error_count < 1000) {
		for (auto &pair : graph) {
			int ver_edges = rand() % (settings.MaxEdgeCount / settings.VertexCount) + 1;
			Vertex* pvert = pair.second;
			cout << endl << "\tvertex: " << pvert->Name << "\n\tto create edges:" << ver_edges << endl;
			while (ver_edges > 0 && total_edge_num > 0 && error_count < 1000) {
				int error;
				Vertex* vertex = chooseToVertex(pvert, settings, graph, error_count, error);
				if (!vertex) {
					last_error = error;
					cout << "Error creating edge for vertex " << pvert->Name << ":";
					switch(error) {
					case FATAL_ERROR_SELF_LOOPS_ARE_PROHIBITED:
						cout << " self-loops are not allowed by settings" << endl;
						break;
					case FATAL_ERROR_BIDIRECTION_EDGES_ARE_PROHIBITED:
						cout << " bidirectional edges are not allowed by settings" << endl;
						break;
					case FATAL_ERROR_EDGE_ALREADY_EXISTS:
						cout << " such edge already exists" << endl;
						break;
					}
					error_count++;
					continue;
				}
				double weight = rand() % (settings.MaxEdgeWeight - settings.MinEdgeWeight + 1) - settings.MinEdgeWeight;
				Edge* edge = addEdge(pvert, vertex, weight, graph, settings);
				if (!edge) {
					last_error = FATAL_ERROR_FAILED_TO_ADD_EDGE;
					error_count++;
					continue;
				}
				ver_edges--;
				total_edge_num--;
			}
		}
	}
	cout << "Error count: " << error_count << "\nLast error: " << last_error << endl;
}

