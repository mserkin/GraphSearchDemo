/*
 * creator.h
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#ifndef INC_CREATOR_H_
#define INC_CREATOR_H_

#include "graph.h"
#include "settings.h"

//Create graph with parameters defined by settings
void createGraph(Graph& graph, const Settings& settings);

//Select random vertex from the graph to which edge could be created
Vertex* chooseToVertex(Vertex *vertex_from, const Settings &settings, Graph &graph,
		int &error_count, int &error);

#endif /* INC_CREATOR_H_ */
