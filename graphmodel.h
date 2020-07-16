#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "graph.h"
#include "algo.h"
#include "searchinfo.h"

enum LoadedGraphType {
    NotLoaded,
    Standard,
    Graph2D
};

class GraphModel
{
public:
    GraphModel(SearchInfo *search_info);
    void clear();
    Graph* getGraph();
    Settings* getSettings();
    void getTextRepresentation(QString& text);
    Vertex* getVertexByName(const QString &name);
    LoadedGraphType getLoadedGraphType();
    int loadFromFile(const QString &filename);
    void prepare();
    int saveToFile(const QString &filename);
private:
    Graph *m_graph = nullptr;
    Settings *m_settings = nullptr;
    LoadedGraphType m_loaded_graph = NotLoaded;
    SearchInfo *m_search_info;
    static void handleAlgorithmEvent(AlgoEvent event, Vertex* vertex, void* user_context);
};

#endif // GRAPHMODEL_H
