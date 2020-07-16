/*
 * specalgo.cpp
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#include "graph.h"
#include "algo.h"
#include "informed.h"


 Vertex2d* Vertex2d::edgelessClone() {
	Vertex2d *clone = new Vertex2d(this->Name, this->X, this->Y);
	return clone;
};


void dijkstra2d(Vertex2d* source, Vertex2d* target, Graph& graph, Callback callback, AlgoResult& result, void* user_context, double coefficient) {
	if (!source || !target) {
		result.ResultCode = NoSourceOrTarget;
		return;
	};

	if (source == target) {
		result.ResultCode = SourceIsTarget;
		return;
	};
	boost::heap::binomial_heap<PVertex, boost::heap::compare<DijkstraVertexComparator>> queue;
	int x_target = target->X, y_target = target->Y;
	DijkstraContext *current_context;
	for (const auto& pair : graph) {
		current_context = new DijkstraContext();
		Vertex* v = pair.second;
		if (v == source) {
			current_context->Weight = 0;
		}
		v->Context = current_context;
		static_cast<DijkstraContext*>(v->Context)->Handle = queue.push(v);
	}

	while (!queue.empty()) {
		Vertex2d* v_current = static_cast<Vertex2d*>(queue.top());
		queue.pop();
		weight_t current_coef = abs(v_current->X - x_target) + abs(v_current->Y - y_target);
		current_context = static_cast<DijkstraContext*>(v_current->Context);
		if (callback) callback(VertexProcessingStarted, v_current, user_context);
		for (const auto &e : *(v_current->OutcomingEdges)) {
			Vertex2d *v_to = static_cast<Vertex2d*>(e->ToVertex);
			DijkstraContext *v_to_context = static_cast<DijkstraContext*>(v_to->Context);
			if (v_to_context->Processed) continue;
			if (callback) callback(VertexDiscovered, v_to, user_context);
			weight_t to_coef = abs(v_to->X - x_target) + abs(v_to->Y - y_target);
			weight_t to_weight_to_be = current_context->Weight + e->Weight + (to_coef - current_coef)*coefficient;
			if (v_to_context->Weight > to_weight_to_be) {
				v_to_context->Weight = to_weight_to_be;
				v_to_context->Parent = static_cast<Vertex*>(v_current);

				queue.increase(static_cast<VertexHandle>(v_to_context->Handle), e->ToVertex);
			}
		}
		current_context->Processed = true;
		if (callback) callback(VertexProcessingFinished, v_current, user_context);

		if (v_current == target) {
			if (callback) callback(TargetFound, v_current, user_context);
			result.ResultCode = Found;
			if (callback) callback(AlgorithmFinished, nullptr, user_context);
			return;
		}
	};

	if (callback) callback(AlgorithmFinished, nullptr, user_context);
	result.ResultCode = NotFound;
}


