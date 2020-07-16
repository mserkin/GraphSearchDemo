#include "searchinfo.h"
#include <stack>

SearchInfo::SearchInfo(/*GraphModel *model*/) //: m_model(model)
{
    m_checked_count = 0;
    m_processed_count = 0;
    m_visited_vertices = new map<const QString, const QPoint*>;
    m_processed_vertices = new map<const QString, const QPoint*>;
    m_selected_algorithm = BellmanFord;
    m_last_run_algorithm = None;
}

void SearchInfo::clear(bool dropSourceTarget) {
    for(auto &pair1 : *m_visited_vertices) delete pair1.second;
    for(auto &pair2 : *m_processed_vertices) delete pair2.second;
    m_visited_vertices->clear();
    m_processed_vertices->clear();
    m_shortest_path.clear();
    m_checked_count = 0;
    m_processed_count = 0;
    if (m_algo_result) {
        delete m_algo_result;
        m_algo_result = nullptr;
    }
    m_algo_result = (m_selected_algorithm == FastDijkstra) ? new BidirectionalDijkstraResult() : new AlgoResult();
    if (dropSourceTarget) {
        m_source_vertex = nullptr;
        m_target_vertex = nullptr;
    }
    notifySubscribers();
}

void SearchInfo::addSubscriber(ISubscriber* subscriber) {
    const auto &it = m_subscribers.find(subscriber);
    if (it == m_subscribers.end()) {
        m_subscribers.insert(subscriber);
    }
};

void SearchInfo::removeSubscriber(ISubscriber* subscriber) {
    const auto &it = m_subscribers.find(subscriber);
    if (it == m_subscribers.end()) {
        m_subscribers.erase(it);     
    }
}

bool SearchInfo::addVisitedVertix(const QString& name, const QPoint* coords) {
    const auto &it = m_visited_vertices->find(name);
    if (it == m_visited_vertices->end()) {
        m_visited_vertices->insert(make_pair(name, coords));
        notifySubscribers();
        return true;
    }
    return false;
}

bool SearchInfo::addProcessedVertix(const QString& name, const QPoint* coords) {
    const auto &it = m_processed_vertices->find(name);
    if (it == m_processed_vertices->end()) {
        m_processed_vertices->insert(make_pair(name, coords));
        notifySubscribers();
        return true;
    }
    return false;
}

void SearchInfo::removeVisitedVertix(const std::_Rb_tree_const_iterator<std::pair<const QString, const QPoint *>> it) {
    m_visited_vertices->erase(it);
    notifySubscribers();
}

void SearchInfo::removeProcessedVertix(const std::_Rb_tree_const_iterator<std::pair<const QString, const QPoint *>> it) {
    m_visited_vertices->erase(it);
    notifySubscribers();
}

const map<const QString, const QPoint*>* SearchInfo::getVisitedVertices() {
    return m_visited_vertices;
}

const map<const QString, const QPoint*>* SearchInfo::getProcessedVertices() {
    return m_processed_vertices;
}

void SearchInfo::notifySubscribers() {
    for (const auto& subscriber : m_subscribers) {
        subscriber->update();
    }
}

Vertex* SearchInfo::getSourceVertex()
{
    return m_source_vertex;
}

Vertex* SearchInfo::getTargetVertex()
{
    return m_target_vertex;
}

void SearchInfo::setSourceVertex(Vertex* source)
{
    m_source_vertex = source;
    notifySubscribers();
}

void SearchInfo::setTargetVertex(Vertex*  target)
{
    m_target_vertex = target;
    notifySubscribers();
}

QString SearchInfo::getSourceVertexName()
{
    if (m_source_vertex) {
        return QString::fromStdString(m_source_vertex->Name);
    }
    else {
        return QString("");
    }
}

QString SearchInfo::getTargetVertexName()
{
    if (m_target_vertex) {
        return QString::fromStdString(m_target_vertex->Name);
    }
    else {
        return QString("");
    }
}

SearchState SearchInfo::getSearchState()
{
    return m_state;
}

void SearchInfo::setSearchState(SearchState state)
{
    m_state = state;
    if (SearchState::Stopped == state && AlgoResultCode::Found == m_algo_result->ResultCode) {
        calculateShortestPath();
    }
    else {
        m_shortest_path.clear();
    }
    notifySubscribers();
}

Algorithm SearchInfo::getSelectedAlgorithm()
{
    return m_selected_algorithm;
}

void SearchInfo::setSelectedAlgorithm(Algorithm algorithm)
{
    m_selected_algorithm = algorithm;
    notifySubscribers();
}

Algorithm SearchInfo::getLastRunAlgorithm()
{
    return m_last_run_algorithm;
}

void SearchInfo::setLastRunAlgorithm(Algorithm algorithm)
{
    m_last_run_algorithm = algorithm;
    notifySubscribers();
}

AlgoResult *SearchInfo::getResult()
{
    return m_algo_result;
}

void SearchInfo::calculateShortestPath()
{
    m_shortest_path.clear();
    Vertex *current_vertex = m_target_vertex;
    BidirectionalDijkstraResult *fast_dijkstra_result = nullptr;
    if (FastDijkstra == m_last_run_algorithm) {
        fast_dijkstra_result = static_cast<BidirectionalDijkstraResult*>(m_algo_result);
        current_vertex = fast_dijkstra_result->ForwardSearchLastVertex;
    }
    stack<Vertex*> path;
    path.push(current_vertex);
    while (current_vertex != m_source_vertex) {
        switch(m_last_run_algorithm) {
        case Dijkstra:
        case Dijkstra2D:
        case BellmanFord:
            current_vertex = static_cast<DijkstraContext*>(current_vertex->Context)->Parent;
            break;
        case BreadthFirstSearch:
        case DepthFirstSearch:
            current_vertex = static_cast<Vertex*>(current_vertex->Context);
            break;
        case FastDijkstra:
            current_vertex = static_cast<BidirectionalDijkstraContext*>(current_vertex->Context)->ParentInForwardSearch;
            break;
        default:
            break;
        }
        path.push(current_vertex);
    }

    while (!path.empty()) {
        m_shortest_path.push_back(path.top());
        path.pop();
    }
    if (m_last_run_algorithm == FastDijkstra) {
        current_vertex = fast_dijkstra_result->BackwardSearchLastVertex;
        m_shortest_path.push_back(current_vertex);
        while (current_vertex != m_target_vertex) {
            current_vertex = static_cast<BidirectionalDijkstraContext*>(current_vertex->Context)->ParentInBackwardSearch;
            m_shortest_path.push_back(current_vertex);
        }
    }
}

const vector<Vertex*>* SearchInfo::getShortestPath()
{
    return &m_shortest_path;
}
