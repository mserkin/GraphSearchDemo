#include <QCoreApplication>
#include "graphcontroller.h"
#include "graph.h"
#include "informed.h"


GraphController* GraphController::instance = nullptr;

void algorithmEventHandler (AlgoEvent event, Vertex* vertex, void* user_context) {
    GraphController::GetInstance(nullptr, nullptr)->handleEvent(event, vertex, user_context);
}

GraphController::GraphController(GraphModel *model, SearchInfo *search_info) : m_model(model), m_search_info(search_info)
{

}

GraphController* GraphController::GetInstance(GraphModel *model, SearchInfo *search_info) {
    if (!instance) {
        instance = new GraphController (model, search_info);
    }
    return instance;
}

void GraphController::runAlgorithm(AlgoRunMode mode)
{
    switch (m_search_info->getSearchState()) {
    case SearchState::RunningStepByStep:
    case SearchState::Running:
        return;
    case SearchState::Stopped:
        if (mode == AlgoRunMode::Uninterrupted) {
            m_search_info->setSearchState(SearchState::Running);
        }
        else {
            m_search_info->setSearchState(SearchState::RunningStepByStep);
        }
        break;
    case SearchState::Paused:
        if (mode == AlgoRunMode::Uninterrupted) {
            m_search_info->setSearchState(SearchState::Running);
        }
        else {
            m_search_info->setSearchState(SearchState::RunningStepByStep);
        }
        return;
    }

    m_model->prepare();
    m_search_info->clear(false);

    Vertex* source_vertex = m_search_info->getSourceVertex();
    Vertex* target_vertex = m_search_info->getTargetVertex();

    AlgoResult *result = m_search_info->getResult();

    m_search_info->setLastRunAlgorithm(m_search_info->getSelectedAlgorithm());
    switch (m_search_info->getSelectedAlgorithm()) {
    case BreadthFirstSearch:
        bfs(source_vertex, target_vertex, algorithmEventHandler, *result);
        break;
    case DepthFirstSearch:
        dfs(source_vertex, target_vertex, algorithmEventHandler, *result);
        break;
    case Dijkstra:
        dijkstra(source_vertex, target_vertex, *m_model->getGraph(), algorithmEventHandler, *result);
        break;
    case BellmanFord:
        bellmanFord(source_vertex, target_vertex, *m_model->getGraph(), algorithmEventHandler, *result);
        break;
    case FastDijkstra: {
        BidirectionalDijkstraResult *fast_dijkstra_result =
                static_cast<BidirectionalDijkstraResult*>(result);
        bidirectionalDijkstra(source_vertex, target_vertex, *m_model->getGraph(), algorithmEventHandler, *fast_dijkstra_result);
        result = fast_dijkstra_result;
        break;
    }
    case Dijkstra2D:
        dijkstra2d(static_cast<Vertex2d*>(source_vertex), static_cast<Vertex2d*>(target_vertex), *m_model->getGraph(), algorithmEventHandler, *result);
        break;
    default: return;
    }
    m_search_info->setSearchState(SearchState::Stopped);
}

void GraphController::handleEvent(AlgoEvent event, Vertex* vertex, void* user_context) {
    (void)user_context;
    Vertex2d *vertex2d = nullptr;
    QString vertex_name;
    if (vertex) {
        vertex_name = QString::fromStdString(vertex->Name);
        if (Graph2D == m_model->getLoadedGraphType())  {
            vertex2d = static_cast<Vertex2d*>(vertex);
        }
    }
    switch(event) {
    case VertexDiscovered:
        m_search_info->m_checked_count++;
        if (vertex2d) {
            m_search_info->addVisitedVertix(vertex_name, new QPoint(vertex2d->X, vertex2d->Y));
        }
        break;
    case VertexProcessingStarted:
        break;
    case VertexProcessingFinished:
        m_search_info->m_processed_count++;
        if (vertex2d) {
            const QPoint *pnt = nullptr;
            const auto &itp_visited = m_search_info->getVisitedVertices()->find(vertex_name);
            if (itp_visited != m_search_info->getVisitedVertices()->end()) {
                pnt = itp_visited->second;
                m_search_info->removeVisitedVertix(itp_visited);
            }
            const auto &itp_processed = m_search_info->getProcessedVertices()->find(vertex_name);
            if (itp_processed == m_search_info->getProcessedVertices()->end()) {
                if (!pnt) {
                    pnt = new QPoint(vertex2d->X, vertex2d->Y);
                }
                m_search_info->addProcessedVertix(vertex_name, pnt);
            }
        }
        break;
    case TargetFound:
        break;
    case TargetNotFound:
        break;
    case AlgorithmFinished:
        break;
    case NegativeLoopDetected:
        break;
    }
    if (SearchState::RunningStepByStep == m_search_info->getSearchState()) {
        m_search_info->setSearchState(SearchState::Paused);
        while (SearchState::RunningStepByStep != m_search_info->getSearchState() &&
               SearchState::Running != m_search_info->getSearchState())  {
            QCoreApplication::processEvents();
        }
    }
}
