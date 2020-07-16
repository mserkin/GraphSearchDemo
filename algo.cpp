/*
 * algo.cpp
 *
 *  Created on: 5 џэт. 2020 у.
 *      Author: Serkin
 */

//#include <iterator>
//#include <algorithm>
#include <iostream>
#include <boost/heap/binomial_heap.hpp>
#include <queue>
#include <stack>
#include "graph.h"
#include "algo.h"
#include "settings.h"

map <AlgoResultCode, string> g_algo_result_text {
	{Ok, "Ok"}, {Found, "Target found"}, {NotFound, "Target not found"}, {NoSourceOrTarget, "Source or target vertex undefined"}
};

string AlgoResult::getText() {
	return g_algo_result_text[this->ResultCode];
}

void bfs(Vertex *source, Vertex *target, Callback callback, AlgoResult& result, void* user_context) {
	if (!source || !target) {
		result.ResultCode = NoSourceOrTarget;
		return;
	};

	if (source == target) {
		result.ResultCode = SourceIsTarget;
		return;
	};

	queue<Vertex*> q;
	q.push(source);
	source->Context = (void*)true;

	while (!q.empty()) {
		Vertex *v = q.front();
		q.pop();
		if (callback) callback(VertexProcessingStarted, v, user_context);
		for (const auto &e : *(v->OutcomingEdges)) {
			if (!e->ToVertex->Context) {
				e->ToVertex->Context = (void*)v;
				q.push(e->ToVertex);
				if (callback) callback(VertexDiscovered, e->ToVertex, user_context);
			}
			if (e->ToVertex == target) {
				if (callback) callback(TargetFound, e->ToVertex, user_context);
				if (callback) callback(AlgorithmFinished, nullptr, user_context);
				result.ResultCode = Found;
				return;
			}
		}
		if (callback) callback(VertexProcessingFinished, v, user_context);
	}
	if (callback) callback(TargetNotFound, nullptr, user_context);
	if (callback) callback(AlgorithmFinished, nullptr, user_context);

    result.ResultCode = NotFound;
}

void dfs(Vertex *source, Vertex *target, Callback callback, AlgoResult& result, void* user_context) {
	if (!source || !target) {
		result.ResultCode = NoSourceOrTarget;
		return;
	};

	if (source == target) {
		result.ResultCode = SourceIsTarget;
		return;
	};

	stack<Vertex*> q;
	q.push(source);
	source->Context = (void*)true;

	while (!q.empty()) {
		Vertex *v = q.top();
		q.pop();
		if (callback) callback(VertexProcessingStarted, v, user_context);
		for (const auto &e : *(v->OutcomingEdges)) {
			if (!e->ToVertex->Context) {
				e->ToVertex->Context = (void*)v;
				q.push(e->ToVertex);
				if (callback) callback(VertexDiscovered, e->ToVertex, user_context);
			}
			if (e->ToVertex == target) {
				if (callback) callback(TargetFound, e->ToVertex, user_context);
				if (callback) callback(AlgorithmFinished, nullptr, user_context);
				result.ResultCode = Found;
				return;
			}
		}
		if (callback) callback(VertexProcessingFinished, v, user_context);
	}
	if (callback) callback(TargetNotFound, nullptr, user_context);
	if (callback) callback(AlgorithmFinished, nullptr, user_context);
    result.ResultCode = NotFound;
}

bool DijkstraVertexComparator::operator()(const PVertex& v1, const PVertex& v2) const {
	if (static_cast<DijkstraContext*>(v1->Context)->Weight > static_cast<DijkstraContext*>(v2->Context)->Weight )
		return true;
	else
		return false;
	// return "true" if "p1" is ordered before "p2"
}

void dijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, AlgoResult& result, void* user_context) {
	if (!source || !target) {
		result.ResultCode = NoSourceOrTarget;
		return;
	};

	if (source == target) {
		result.ResultCode = SourceIsTarget;
		return;
	};

	boost::heap::binomial_heap<PVertex, boost::heap::compare<DijkstraVertexComparator>> queue;
	DijkstraContext *current_vertex_context;
	for (const auto& pair : graph) {
		Vertex *v = pair.second;
		current_vertex_context = new DijkstraContext();
		if (v == source) {
			current_vertex_context->Weight = 0;
		}
		v->Context = current_vertex_context;
		static_cast<DijkstraContext*>(v->Context)->Handle = queue.push(v);
	}

	Vertex* v;
	while (!queue.empty()) {
		v = queue.top();
		queue.pop();
		current_vertex_context = static_cast<DijkstraContext*>(v->Context);

		//Unreachable vertex was taken! Thus there is no more unprocessed vertices that can be reached from source
		if(current_vertex_context->Weight == INFINITE_WEIGHT) {
			break;
		}

		if (callback) callback(VertexProcessingStarted, v, user_context);
		for (const auto &e : *(v->OutcomingEdges)) {
			DijkstraContext *neightbor_vertex_context = static_cast<DijkstraContext*>(e->ToVertex->Context);
			if (neightbor_vertex_context->Processed) continue;
			if (callback) callback(VertexDiscovered, e->ToVertex, user_context);

			weight_t new_weight = current_vertex_context->Weight + e->Weight;
			if (neightbor_vertex_context->Weight > new_weight) {
				neightbor_vertex_context->Weight = new_weight;
				neightbor_vertex_context->Parent = v;

				queue.increase(static_cast<VertexHandle>(neightbor_vertex_context->Handle), e->ToVertex);
			}
		}
		static_cast<DijkstraContext*>(v->Context)->Processed = true;
		if (callback) callback(VertexProcessingFinished, v, user_context);

		if (v == target) {
			if (callback) callback(TargetFound, v, user_context);
			result.ResultCode = Found;
			if (callback) callback(AlgorithmFinished, nullptr, user_context);
			return;
		}
	};

	if (callback) callback(AlgorithmFinished, nullptr, user_context);
	result.ResultCode = NotFound;
}

void bellmanFord(Vertex* source, Vertex* target, Graph& graph, Callback callback, AlgoResult& result, void* user_context) {
	if (!source || !target) {
		result.ResultCode = NoSourceOrTarget;
		return;
	};

	if (source == target) {
		result.ResultCode = SourceIsTarget;
		return;
	};

	DijkstraContext *current_vertex_context;
	for (const auto& pair1 : graph) {
		Vertex *v = pair1.second;
		current_vertex_context = new DijkstraContext();
		if (v == source) {
			current_vertex_context->Weight = 0;
		}
		v->Context = current_vertex_context;
	}

	//n+1 times as last time is for negative loop check
	for (std::vector<Vertex*>::size_type i = 0; i <= graph.size(); i++) {
		for (const auto& pair2 : graph) {
			Vertex *vertex = pair2.second;
			current_vertex_context = static_cast<DijkstraContext*>(vertex->Context);
			for (const auto &e : *(vertex->OutcomingEdges)) {
				DijkstraContext *neighbor_vertex_context = static_cast<DijkstraContext*>(e->ToVertex->Context);
				weight_t new_weight = current_vertex_context->Weight + e->Weight;
				if (neighbor_vertex_context->Weight > new_weight) {
					neighbor_vertex_context->Weight = new_weight;
					neighbor_vertex_context->Parent = vertex;
					if (i == graph.size()) {
						if (callback) callback(NegativeLoopDetected, vertex, user_context);
						if (callback) callback(AlgorithmFinished, nullptr, user_context);
						result.ResultCode = NegativeLoopFound;
						return;
					}
				}
			}
		}
	}

	if (callback) callback(AlgorithmFinished, nullptr, user_context);
	result.ResultCode = (static_cast<DijkstraContext*>(target->Context)->Weight < INFINITE_WEIGHT) ? Found : NotFound;
}

bool FastDijkstraForwardComparator::operator()(const PVertex& v1, const PVertex& v2) const {
	return (static_cast<BidirectionalDijkstraContext*>(v1->Context)->WeightInForwardSearch >
		static_cast<BidirectionalDijkstraContext*>(v2->Context)->WeightInForwardSearch );
	// return "true" if "p1" is ordered before "p2"
}

bool FastDijkstraBackwardComparator::operator()(const PVertex& v1, const PVertex& v2) const {
	return (static_cast<BidirectionalDijkstraContext*>(v1->Context)->WeightInBackwardSearch >
		static_cast<BidirectionalDijkstraContext*>(v2->Context)->WeightInBackwardSearch );
	// return "true" if "p1" is ordered before "p2"
}

void bidirectionalDijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, BidirectionalDijkstraResult& result, void* user_context) {
	if (!source || !target) {
		result.ResultCode = NoSourceOrTarget;
		return;
	};

	if (source == target) {
		result.ResultCode = SourceIsTarget;
		return;
	};

	boost::heap::binomial_heap<PVertex, boost::heap::compare<FastDijkstraForwardComparator>> forward_queue;
	boost::heap::binomial_heap<PVertex, boost::heap::compare<FastDijkstraBackwardComparator>> backward_queue;
	BidirectionalDijkstraContext *current_vertex_context;
	result.ResultCode = NotFound;
	//initialize all vertices' contexts
	for (const auto& pair : graph) {
		Vertex* v = pair.second;
		current_vertex_context = new BidirectionalDijkstraContext();
		v->Context = current_vertex_context;
		if (v == source) {
			current_vertex_context->WeightInForwardSearch = 0;
		}
		if (v == target) {
			current_vertex_context->WeightInBackwardSearch = 0;
		}
		current_vertex_context->HandleInForwardSearch = forward_queue.push(v);
		current_vertex_context->HandleInBackwardSearch = backward_queue.push(v);
	}
	
	weight_t shortest_path_weight = INFINITE_WEIGHT, new_path_weight;
	Vertex *current_forward_search_vertex, *current_backward_search_vertex, *neighbor;
	Vertex *forward_shortest_path_vertex = nullptr, *backward_shortest_path_vertex = nullptr;
	BidirectionalDijkstraContext *neighbor_context;
	weight_t shortest_path_bridge_weight = INFINITE_WEIGHT;
	while (!forward_queue.empty() && !backward_queue.empty()) {
		current_forward_search_vertex = forward_queue.top();
		current_backward_search_vertex = backward_queue.top();
		
		double best_forward_search = static_cast<BidirectionalDijkstraContext*>(forward_queue.top()->Context)->WeightInForwardSearch;
		double best_backward_search = static_cast<BidirectionalDijkstraContext*>(backward_queue.top()->Context)->WeightInBackwardSearch;
		//If path found and there no candidates to be shortest
		if (shortest_path_weight < INFINITE_WEIGHT && shortest_path_weight < best_forward_search + best_backward_search)
		{
			if (callback) callback(AlgorithmFinished, nullptr, user_context);
			result.ForwardSearchLastVertex = forward_shortest_path_vertex;
			result.BackwardSearchLastVertex = backward_shortest_path_vertex;
			result.ConnectingEdgeWeight = shortest_path_bridge_weight;
			result.ResultCode = Found;
			return;
		}
		
		//process current vertex in forward search
		if (!forward_queue.empty()) {
			current_vertex_context = static_cast<BidirectionalDijkstraContext*>(current_forward_search_vertex->Context);
			if (callback) callback(VertexProcessingStarted, current_forward_search_vertex, user_context);
		
			//process all outgoing edges from current forward search vertex
			for (const auto &fe : *(current_forward_search_vertex->OutcomingEdges)) {
				neighbor = fe->ToVertex;
				neighbor_context = static_cast<BidirectionalDijkstraContext*>(neighbor->Context);
				//if this vertex is already processed no need to process it again
				if (neighbor_context->ProcessedByForwardSearch) continue;
				if (callback) callback(VertexDiscovered, neighbor, user_context);
				
				//release edge - update neighbor vertex weight and parent
				if (neighbor_context->WeightInForwardSearch > current_vertex_context->WeightInForwardSearch + fe->Weight) {
					neighbor_context->WeightInForwardSearch = current_vertex_context->WeightInForwardSearch + fe->Weight;
					neighbor_context->ParentInForwardSearch = current_forward_search_vertex;

					forward_queue.increase(static_cast<ForwardSearchVertexHandle>(neighbor_context->HandleInForwardSearch), neighbor);
				}

				//if neighbor vertex is already processed by backward search check weather we have new shortest path!
				if (neighbor_context->ProcessedByBackwardSearch) {
					new_path_weight = neighbor_context->WeightInBackwardSearch + fe->Weight + current_vertex_context->WeightInForwardSearch;
					if (new_path_weight < shortest_path_weight) {
						shortest_path_weight = new_path_weight;
						forward_shortest_path_vertex = current_forward_search_vertex;
						backward_shortest_path_vertex = neighbor;
						shortest_path_bridge_weight = fe->Weight;
					}
				}
			}
			current_vertex_context->ProcessedByForwardSearch = true;
			if (callback) callback(VertexProcessingFinished, current_forward_search_vertex, user_context);

			forward_queue.pop();
		};

		if (shortest_path_weight < INFINITE_WEIGHT && shortest_path_weight < best_forward_search + best_backward_search)
		{
			if (callback) callback(AlgorithmFinished, nullptr, user_context);
			result.ForwardSearchLastVertex = forward_shortest_path_vertex;
			result.BackwardSearchLastVertex = backward_shortest_path_vertex;
			result.ConnectingEdgeWeight = shortest_path_bridge_weight;
			result.ResultCode = Found;
			return;
		}

		//process current vertex in forward search
		if (!backward_queue.empty()) {
			current_vertex_context = static_cast<BidirectionalDijkstraContext*>(current_backward_search_vertex->Context);
			if (callback) callback(VertexProcessingStarted, current_backward_search_vertex, user_context);
		
			//process all incoming edges to current backward search vertex
			for (const auto &be : *(current_backward_search_vertex->IncomingEdges)) {
				neighbor = be->FromVertex;
				neighbor_context = static_cast<BidirectionalDijkstraContext*>(neighbor->Context);
				//if this vertex is already processed no need to process it again
				if (neighbor_context->ProcessedByBackwardSearch) continue;
				if (callback) callback(VertexDiscovered, neighbor, user_context);
				
				//release edge - update neighbor vertex weight and parent
				if (neighbor_context->WeightInBackwardSearch > current_vertex_context->WeightInBackwardSearch + be->Weight) {
					neighbor_context->WeightInBackwardSearch = current_vertex_context->WeightInBackwardSearch + be->Weight;
					neighbor_context->ParentInBackwardSearch = current_backward_search_vertex;

					backward_queue.increase(static_cast<BackwardSearchVertexHandle>(neighbor_context->HandleInBackwardSearch), neighbor);
				}

				//if neighbor vertex is already processed by forward search check weather we have new shortest path!
				if (neighbor_context->ProcessedByForwardSearch) {
					new_path_weight = neighbor_context->WeightInForwardSearch + be->Weight + current_vertex_context->WeightInBackwardSearch;
					if (new_path_weight < shortest_path_weight) {
						shortest_path_weight = new_path_weight;
						forward_shortest_path_vertex = neighbor;
						backward_shortest_path_vertex = current_backward_search_vertex;
						shortest_path_bridge_weight = be->Weight;
					}
				}
			}
			current_vertex_context->ProcessedByBackwardSearch = true;
			if (callback) callback(VertexProcessingFinished, current_backward_search_vertex, user_context);

			backward_queue.pop();
		};
	}
	if (callback) callback(AlgorithmFinished, nullptr, user_context);
};

void clearContext (Algorithm algo, Graph& graph) {
	for (auto &pair : graph) {
		Vertex *vertex = pair.second;
		switch (algo) {
		case Dijkstra:
		case BellmanFord:
		case Dijkstra2D:
			delete static_cast<DijkstraContext*>(vertex->Context);
			break;
		case FastDijkstra:
			delete static_cast<BidirectionalDijkstraContext*>(vertex->Context);
			break;
		default: break;
		}
		vertex->Context = nullptr;
	}
}
