/*
 * algo.h
 */

#ifndef ALGO_H_
#define ALGO_H_

#include <boost/heap/binomial_heap.hpp>
#include <map>
#include "graph.h"

enum AlgoEvent {
	VertexDiscovered,            //Vertex were inspected but not yet fully processed
	VertexProcessingStarted,     //Start to process vertex
	VertexProcessingFinished,    //Vertex processing finished
	NegativeLoopDetected,        //Algorithm detected negative cycle
	TargetFound,                 //Target vertex was found during search from source vertex
	TargetNotFound,              //Processing finished but target vertex was not found
	AlgorithmFinished            //Finished executing the algorithm
};

enum AlgoResultCode {
	Ok,							//Finished executing the algorithm
	Found,                      //Found a path, shortest path or minimal-weight path
	NotFound,                   //No paths from source to target was not found
	NoSourceOrTarget,           //Source or target vertex was not found in graph
	SourceIsTarget,             //Source and target are the same vertex
	NegativeLoopFound		    //Negative loop was detected and algorithm stopped
};

struct AlgoResult {
	AlgoResultCode ResultCode = NotFound;
	string getText();
};

struct DijkstraContext;

struct DijkstraVertexComparator {  //Compare functor that is used in Dijkstra algorithm
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

struct FastDijkstraForwardComparator { //Compare functor that is used in bidirectional Dijkstra algorithm in forward search
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

struct FastDijkstraBackwardComparator { //Compare functor that is used in bidirectional Dijkstra algorithm in backward search
    bool operator()(const PVertex& v1, const PVertex& v2) const;
};

typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<DijkstraVertexComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> VertexHandle;
typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<FastDijkstraForwardComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> ForwardSearchVertexHandle;
typedef boost::heap::detail::node_handle<boost::heap::detail::parent_pointing_heap_node<PVertex>*, boost::heap::detail::make_binomial_heap_base<PVertex, boost::parameter::aux::flat_like_arg_list<boost::parameter::aux::flat_like_arg_tuple<boost::heap::tag::compare, boost::heap::compare<FastDijkstraBackwardComparator>, std::integral_constant<bool, true> > > >::type, PVertex&> BackwardSearchVertexHandle;

typedef double weight_t;
#define INFINITE_WEIGHT 1E30

struct BidirectionalDijkstraResult : AlgoResult{
	Vertex* ForwardSearchLastVertex = nullptr;     //Last vertex of the minimal weight path in forward search (before connecting edge)
	Vertex* BackwardSearchLastVertex = nullptr;    //Last vertex of the minimal weight path in backward search (after connecting edge)
	weight_t ConnectingEdgeWeight = INFINITE_WEIGHT; //Weight of the edge that connects ForwardSearchLastVertex and BackwardSearchLastVertex
	BidirectionalDijkstraResult() {
		ResultCode = NotFound;
	}
};

struct DijkstraContext {   //Vertex additional information for Dijkstra algorithm.
	weight_t Weight = INFINITE_WEIGHT;  //Weight of the minimal-weight path from source to that vertex
	Vertex* Parent = nullptr;           //Previous vertex in the minimal-weight path from source to that vertex
	VertexHandle Handle;                //Handle of the vertex to be used in priority queue (benomial heap)
	bool Processed = false;             //Vertex processing completion mark
};

struct BidirectionalDijkstraContext { //Vertex additional information for bidirectional Dijkstra algorithm.
	weight_t WeightInForwardSearch = INFINITE_WEIGHT;  //Weight of the minimal-weight path from source to that vertex
	Vertex* ParentInForwardSearch = nullptr;           //Previous vertex in the minimal-weight path from source to that vertex
	ForwardSearchVertexHandle HandleInForwardSearch;   //Handle of the vertex to be used in forward search priority queue (benomial heap)
	bool ProcessedByForwardSearch = false; 			   //Vertex forward search processing completion mark
	weight_t WeightInBackwardSearch = INFINITE_WEIGHT; //Weight of the minimal-weight path from target to that vertex
	Vertex* ParentInBackwardSearch = nullptr;          //Previous vertex in the minimal-weight path from target to that vertex
	BackwardSearchVertexHandle HandleInBackwardSearch; //Handle of the vertex to be used in backward search priority queue (benomial heap)
	bool ProcessedByBackwardSearch = false;            //Vertex backward search processing completion mark
	BidirectionalDijkstraContext(){};
};

//Callback prototype
//event - event type
//vertex - vertex the event is about
//user_context - the context that was set when the algorithm started.
//       it may be needed to understand the origin of event if several algorithm launches were made simultaneously
typedef void (*Callback)(AlgoEvent event, Vertex* vertex, void* user_context);


//Breadth-first search. Finds the shortest path from source to target in the sense of the minimum number of transitions between vertices.
//The algorithm does not consider (ignores) the weight of the edges.
//Complexity O(|V| + |E|)
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution: Found - path from source to target was found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path:
//target->Context will contain a pointer to a previous vertex. The previous vertex's Context field will pointer to it's previous vertex
//and so until source is met.
void bfs(Vertex* source, Vertex* target, Callback callback, AlgoResult& result, void* user_context = nullptr);

//Depth-first search. Finds any path from source to target
//Complexity O(|V| + |E|)
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution: Found - path from source to target was found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path:
//target->Context will contain a pointer to a previous vertex. The previous vertex's Context field will pointer to it's previous vertex
//and so until source is met.
void dfs(Vertex* source, Vertex* target, Callback callback, AlgoResult& result, void* user_context = nullptr);

//Dijkstra algorithm implementation. Finds the minimum weight path from source to target in a weighted graph.
//Complexity O(|E|+|V|*log(|V|)
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution: Found - path from source to target found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path:
//    static_cast<DijkstraContext*>(target->Context)->Parent will contain a pointer to a previous vertex.
//    Do with it the same to get it's previous vertex. Do so until source is met.
void dijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback,  AlgoResult& result, void* user_context = nullptr);

//Bidirectional Dijkstra algorithm implementation. Finds the minimum weight path from source to target in a weighted graph.
//The search is performed from source to target and from target to source using incoming edges - Vertex::IncomingEdged should be provided for each vertex.
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution: Found - path from source to target found or NotFound if no path from source to target was found
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path:
//    path is represented by two parts:
//        1) result.ForwardSearchLastVertex will contain the last vertex in minimum weight path that was found by forward search.
//        go from it to source (opposite direction):
//		     get previous vertex: static_cast<BidirectionalDijkstraContext*>(result.ForwardSearchLastVertex->Context)->ParentInForwardSearch
//           do so until the source is met
//        2) result.BackwardSearchLastVertex will contain the last vertex in minimum weight path that was found by backward search.
//        go from it to target (the direction will be the same as of path itself):
//		     get previous vertex: static_cast<BidirectionalDijkstraContext*>(result.BackwardSearchLastVertex->Context)->ParentInBackwardSearch
//           do so until the target is met
void bidirectionalDijkstra(Vertex* source, Vertex* target, Graph& graph, Callback callback, BidirectionalDijkstraResult& result, void* user_context = nullptr);

//Bellman-Ford algorithm implementation. Finds the minimum weight path from single source to *ALL* other vertices in a weighted graph.
//Found paths and its weights are accessible using context information associated to vertices after run.
//Do not send VertexDiscovered, VertexProcessingStarted and VertexProcessingFinished events as all vertices processed several times
//Sends NegativeLoopDetected with vertex where negative loop was detected
//Complexity O(|V||E|)
//source - source vertex
//target - target vertex
//callback - function that is called by algorithm to supple events to caller
//result - result of algorithm execution:
//          Found - path from source to target found
//          NotFound - no path from source to target was found
//          NegativeLoopFound - negative loop was detected and algorithm stopped. Results (vertex context information) are undefined.
//user_context - pointer to some info - may be needed to understand the origin of event if several algorithm
//               launches were made simultaneously
//Way to get found path is the same as for Dijkstra algorithm
void bellmanFord(Vertex* source, Vertex* target, Graph& graph, Callback callback,  AlgoResult& result, void* user_context = nullptr);
#endif /* ALGO_H_ */

//Delete all context allocated during algorithm work. Should be called after results of algorithm are no more needed
//and before next algorithm call on the same graph.
//Graph& graph - graph
void clearContext (Algorithm algo, Graph& graph);
