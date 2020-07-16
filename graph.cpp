/*
 * graph.cpp
 *
 *  Created on: 3 џэт. 2020 у.
 *      Author: Serkin
 */
#include "graph.h"

Vertex::Vertex(string _Name): Name(_Name) {
	IncomingEdges = new EdgeList();
	OutcomingEdges = new EdgeList();
};

Vertex::~Vertex() {
	PEdge pedge;
	while (!this->OutcomingEdges->empty()) {
		//take last
		auto it1 = this->OutcomingEdges->end();
		pedge = *(--it1);
		//remove last
		this->OutcomingEdges->pop_back();
		//find and remove edge from incomings
		for (it1 = pedge->ToVertex->IncomingEdges->begin(); it1 !=pedge->ToVertex->IncomingEdges->end(); it1++) {
			if ((*it1)->FromVertex == this) {
				pedge->ToVertex->IncomingEdges->erase(it1);
				break;
			}
		}
		delete pedge;
	}
	while (!this->IncomingEdges->empty()) {
		//take last
		auto it2 = this->IncomingEdges->end();
		pedge = *(--it2);
		//remove last
		this->IncomingEdges->pop_back();
		//find edge in outcomings
		for (it2 = pedge->FromVertex->OutcomingEdges->begin(); it2 !=pedge->FromVertex->OutcomingEdges->end(); it2++) {
			if ((*it2)->ToVertex == this) {
				//Erase it :)
				pedge->FromVertex->OutcomingEdges->erase(it2);
				break;
			}
		}
		delete pedge;
	}

	delete OutcomingEdges;
	delete IncomingEdges;
}

Vertex *Vertex::edgelessClone() {
	Vertex *clone = new Vertex(this->Name);
	return clone;
};

Edge* addEdge (Vertex *from, Vertex *to, const double weight, Graph &graph, const Settings& settings) {
	if (!from || !to) return nullptr;
	if (!findVertex(from->Name, graph) || !findVertex(to->Name, graph)) return nullptr;
	if (from == to && !settings.SelfLoop)  return nullptr;
	if (!settings.BiDirectional) {
		for (auto it = to->OutcomingEdges->begin(); it != to->OutcomingEdges->end(); it++) {
			if ((*it)->ToVertex == from) return nullptr;
		}
	}
	if ((settings.MinEdgeWeight != 0.0 || settings.MaxEdgeWeight != 0.0)
			&& (weight < settings.MinEdgeWeight || weight > settings.MaxEdgeWeight)) {
		return nullptr;
	}

	Edge *edge = new Edge(from, to, weight);
	from->OutcomingEdges->push_back(edge);
	to->IncomingEdges->push_back(edge);
	return edge;
}

bool addVertex (Vertex *vertex, Graph &graph, const Settings& settings) {
	if (findVertex(vertex->Name, graph)) {
		return false;
	}
	for (auto ite = vertex->OutcomingEdges->begin(); ite != vertex->OutcomingEdges->end(); ite++) {
		if (!findVertex((*ite)->ToVertex->Name, graph)) return false;
		if ((*ite)->ToVertex == (*ite)->FromVertex && !settings.SelfLoop) return false;
		if ((*ite)->Weight > settings.MaxEdgeWeight || (*ite)->Weight < settings.MinEdgeWeight) return false;
	}
	graph[vertex->Name] = vertex;
	return true;
}

Vertex* addVertex (const string &name, Graph &graph, const Settings& settings) {
	Vertex *vertex = new Vertex (name);
	bool success = addVertex(vertex, graph, settings);
	if (success) {
		return vertex;
	}
	else {

		return nullptr;
	}
}

void removeVertex (Vertex **ppvertex, Graph &graph) {
	PVertex pvertex = (*ppvertex);
	graph.erase(pvertex->Name);
	delete pvertex;
	ppvertex = nullptr;
}

Vertex* findVertex(const string &name, const Graph &graph) {
	auto found = graph.find(name);
	if (found != graph.end()) {
		return found->second;
	}
	else {
		return nullptr;
	}
}

Graph& cloneGraph(const Graph &graph) {
	Graph *clone_graph = new Graph();
	for (auto& pair : graph) {
		Vertex *original_vertex = pair.second;
		Vertex *clone_vertex = findVertex(original_vertex->Name, *clone_graph);
		if (!clone_vertex) {
			clone_vertex = original_vertex->edgelessClone();
			(*clone_graph)[pair.first] = clone_vertex;
		}

		for (auto original_edge : *original_vertex->OutcomingEdges) {
			Vertex *original_target_vertex = original_edge->ToVertex;
			Vertex *clone_target_vertex = findVertex(original_target_vertex->Name, *clone_graph);
			if (!clone_target_vertex) {
				clone_target_vertex = original_target_vertex->edgelessClone();
				(*clone_graph)[original_target_vertex->Name] = clone_target_vertex;
			}
			Edge *clone_edge = new Edge(clone_vertex, clone_target_vertex, original_edge->Weight);
			clone_vertex->OutcomingEdges->push_back(clone_edge);
			clone_target_vertex->IncomingEdges->push_back(clone_edge);
		}
	}
	return *clone_graph;
}

void clearGraph(Graph &graph) {
	for (auto& pair : graph) {
		delete pair.second;
	}
	graph.clear();
}
