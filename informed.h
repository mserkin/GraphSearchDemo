/*
 * specalgo.h
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#ifndef INC_INFORMED_H_
#define INC_INFORMED_H_

#include <string>
#include "graph.h"
#include "algo.h"

struct Vertex2d : public Vertex {
	int X;
	int Y;
	Vertex2d(string _Name, int _X,  int _Y): Vertex(_Name), X(_X), Y(_Y) {};
	virtual Vertex2d* edgelessClone() override;
};


//Dijkstra implementation to be used to find the shortest path in non-weighted graphs where vertices have 2 independent (x and y) coordinates.
//Coordinates are used as heuristic to guide it's search = vertices that closer to target in terms of coordinates have an advantage.
//All edges are recommended to have the weight of 1.
//Weight of the next vertex is calculated as weight_of_current_vertex + weight_of_edge + (distance_from_next_to_target - distance_from_current_to_target)*coefficient
//Parameters:
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to send events to caller
//result - result of algorithm execution: Found - path from source to target found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//coefficient - coefficient to use in weight calculation (see formula above)
//Way to get found path - the same as for Dijkstra.
void dijkstra2d(Vertex2d* source, Vertex2d* target, Graph& graph, Callback callback,  AlgoResult& result, void* user_context = nullptr, double coefficient = 0.5);


#endif /* INC_INFORMED_H_ */
