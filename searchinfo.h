#ifndef PROGRESSINFO_H
#define PROGRESSINFO_H

#include <map>
#include <set>
#include <QString>
#include <QPoint>
#include "settings.h"
#include "graph.h"
#include "algo.h"

enum class SearchState {
    Stopped,
    Running,
    RunningStepByStep,
    Paused
};

class ISubscriber {
public:
    virtual void update() = 0;
};

class GraphModel;

struct SearchInfo
{
private:
    set<ISubscriber*> m_subscribers;
    map<const QString, const QPoint*> *m_visited_vertices;
    map<const QString, const QPoint*> *m_processed_vertices;
    Vertex* m_source_vertex = nullptr;
    Vertex* m_target_vertex = nullptr;
    SearchState m_state = SearchState::Stopped;
    Algorithm m_selected_algorithm;
    Algorithm m_last_run_algorithm;
    AlgoResult* m_algo_result = nullptr;
    vector<Vertex*> m_shortest_path;
    //GraphModel *m_model;
public:
    int m_checked_count;
    int m_processed_count;

    SearchInfo(/*GraphModel *model*/);
    void clear(bool dropSourceTarget = true);
    bool addProcessedVertix(const QString& name, const QPoint* coords);
    void addSubscriber(ISubscriber* subscriber);
    bool addVisitedVertix(const QString& name, const QPoint* coords);
    void calculateShortestPath();
    Algorithm getLastRunAlgorithm();
    AlgoResult* getResult();
    SearchState getSearchState();
    Algorithm getSelectedAlgorithm();
    const vector<Vertex*>* getShortestPath();
    Vertex* getSourceVertex();
    QString getSourceVertexName();
    Vertex* getTargetVertex();
    QString getTargetVertexName();
    void setSelectedAlgorithm(Algorithm algorithm);
    void setLastRunAlgorithm(Algorithm algorithm);
    void setSearchState(SearchState state);
    void setSourceVertex(Vertex *source);
    void setTargetVertex(Vertex *target);
    void notifySubscribers();
    void removeProcessedVertix(const std::_Rb_tree_const_iterator<std::pair<const QString, const QPoint *>> it);
    void removeSubscriber(ISubscriber* subscriber);
    void removeVisitedVertix(const std::_Rb_tree_const_iterator<std::pair<const QString, const QPoint *>> it);
    const map<const QString, const QPoint*>* getVisitedVertices();
    const map<const QString, const QPoint*>* getProcessedVertices();
};

#endif // PROGRESSINFO_H
