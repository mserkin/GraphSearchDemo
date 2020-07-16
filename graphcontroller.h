#ifndef GRAPHCONTROLLER_H
#define GRAPHCONTROLLER_H

#include "graphmodel.h"

enum class AlgoRunMode {
    Uninterrupted,
    StepByStep
};

class GraphController
{
private:
    GraphModel *m_model = nullptr;
    SearchInfo *m_search_info = nullptr;
    Algorithm m_last_run_algo = None;
    static GraphController* instance;
    GraphController(GraphModel *model, SearchInfo *search_info);
public:
    static GraphController* GetInstance(GraphModel *model, SearchInfo *search_info);
    GraphController(GraphController &other) = delete;
    void operator=(const GraphController&) = delete;

    void runAlgorithm(AlgoRunMode mode);
    void handleEvent(AlgoEvent event, Vertex* vertex, void* user_context);
};

#endif // GRAPHCONTROLLER_H
