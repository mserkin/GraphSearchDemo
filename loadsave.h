/*
 * loadsave.h
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#ifndef INC_LOADSAVE_H_
#define INC_LOADSAVE_H_

#include "graph.h"
#include "settings.h"

//Load graph from the file of json format
void loadGraph(Graph& graph, const Settings& settings);

//Load graph from the file of 2d format
int load2dGraph(Graph& graph, const Settings& settings);

//Save graph from the file of json format
void saveGraph(Graph& graph, const Settings& settings);



#endif /* INC_LOADSAVE_H_ */
