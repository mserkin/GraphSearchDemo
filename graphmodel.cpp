#include "graph.h"
#include "graphmodel.h"
#include "loadsave.h"
#include <QTextStream>

GraphModel::GraphModel(SearchInfo *search_info): m_search_info(search_info)
{
    m_settings = new Settings;
    m_settings->Verbose = true;
    m_settings->SelfLoop = true;
    m_settings->BiDirectional = true;
    m_graph = new Graph();
}

Graph* GraphModel::getGraph() {
    return m_graph;
}

void GraphModel::prepare() {
    clearContext(m_search_info->getLastRunAlgorithm(), *m_graph);
}

void GraphModel::clear() {
    prepare();
    clearGraph(*m_graph);
}

Vertex* GraphModel::getVertexByName(const QString &name) {
    return findVertex(name.toStdString(), *m_graph);
}

int GraphModel::loadFromFile(const QString &filename) {
    clear();
    m_search_info->clear();

    m_settings->FilePath.assign(filename.toStdString());
    m_settings->LoadFromFile = true;
    if (filename.endsWith(QString::fromStdString(".2d"))) {
        int err = load2dGraph(*m_graph, *m_settings);
        if (err < 0) {
            m_loaded_graph = NotLoaded;
            return err;
        }
        else
            m_loaded_graph = Graph2D;
    }
    else if (filename.endsWith(QString::fromStdString(".json"))) {
        //Load graph from the file of json format
        loadGraph(*m_graph, *m_settings);
        m_loaded_graph = Standard;
    }
    m_search_info->notifySubscribers();
    return 0;
}

LoadedGraphType GraphModel::getLoadedGraphType() {
    return m_loaded_graph;
}

void GraphModel::getTextRepresentation(QString& text) {
    text.clear();
    QTextStream stm_graph(&text);
    for (auto& pair: *m_graph) {
        stm_graph << pair.first.c_str() << "->";
        for (auto& edge: *pair.second->OutcomingEdges) {
            stm_graph << edge->ToVertex->Name.c_str() << "; ";
        }
        stm_graph << "\n";
    }
}

Settings *GraphModel::getSettings()
{
    return m_settings;
}

int GraphModel::saveToFile(const QString &filename)
{
    m_settings->FilePath.assign(filename.toStdString());
    m_settings->SaveToFile = true;

    if (filename.endsWith(QString::fromStdString(".2d"))) {
        /*
        int err = save2dGraph(*m_graph, *m_settings);
        if (err < 0) {
            m_loaded_graph = NotLoaded;
            showError("Failed to save graph);
            return err;
        }
        else
            m_loaded_graph = Graph2D;
         */
    }
    else if (filename.endsWith(QString::fromStdString(".json"))) {
        //Save graph to the file of json format
        saveGraph(*m_graph, *m_settings);
        m_loaded_graph = Standard;
    }
    return 0;
}
