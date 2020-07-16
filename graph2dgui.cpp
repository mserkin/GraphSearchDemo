#include <QPainter>
#include <QRect>
#include <QPaintEvent>
#include <QShowEvent>
#include <QColor>
#include <QCoreApplication>
//#include <>
//#include <>
//#include <>
//#include <>
#include "graph.h"
#include "informed.h"
#include "graph2dgui.h"

const int VERTEX_SELECTOR_SIDE_LENGTH = 32;
const int VERTEX_SPACING = 4;
const int VERTEX_X_COUNT = 9;
const int VERTEX_Y_COUNT = 9;
const int VERTEX_INDICATOR_SIDE_LENGTH = 8;
const int VERTEX_PROCESSING_MARK_SIDE_LENGTH = 16;
const int INDICATOR_OFFSET = (VERTEX_SELECTOR_SIDE_LENGTH - VERTEX_INDICATOR_SIDE_LENGTH)/2;
const int PROC_MARK_OFFSET = (VERTEX_SELECTOR_SIDE_LENGTH - VERTEX_PROCESSING_MARK_SIDE_LENGTH)/2;

void getVertexNameByCoords(int x, int y, QString &str) {
    str.clear();
    str.append(" : ");
    str[0] = x + 0x30;
    str[2] = y + 0x30;
}

void getVertexBoxInfo(int vertexX, int vertexY, QRect *Indicator = nullptr, QRect *Selector = nullptr, QRect *ProcMark = nullptr) {
    int areaLeft {vertexX*(VERTEX_SELECTOR_SIDE_LENGTH+VERTEX_SPACING)};
    int areaTop {vertexY*(VERTEX_SELECTOR_SIDE_LENGTH+VERTEX_SPACING)};

    if (Selector) {
        Selector->setLeft(areaLeft);
        Selector->setTop(areaTop);
        Selector->setWidth(VERTEX_SELECTOR_SIDE_LENGTH);
        Selector->setHeight(VERTEX_SELECTOR_SIDE_LENGTH);
    }
    if (Indicator) {
        Indicator->setX(areaLeft + INDICATOR_OFFSET);
        Indicator->setY(areaTop + INDICATOR_OFFSET);
        Indicator->setWidth(VERTEX_INDICATOR_SIDE_LENGTH);
        Indicator->setHeight(VERTEX_INDICATOR_SIDE_LENGTH);
    }
    if (ProcMark) {
        ProcMark->setX(areaLeft + PROC_MARK_OFFSET);
        ProcMark->setY(areaTop + PROC_MARK_OFFSET);
        ProcMark->setWidth(VERTEX_PROCESSING_MARK_SIDE_LENGTH);
        ProcMark->setHeight(VERTEX_PROCESSING_MARK_SIDE_LENGTH);
    }
}

void getVertexCoordsByFormCoords(int xCoord, int yCoord, int &vertexX, int &vertexY) {
    vertexX = xCoord / (VERTEX_SELECTOR_SIDE_LENGTH+VERTEX_SPACING);
    vertexY = yCoord / (VERTEX_SELECTOR_SIDE_LENGTH+VERTEX_SPACING);
}

bool linkToNeighborVertex (Vertex2d* vertex, int neighbor_x, int neighbor_y,
                           int left_bound, int right_bound, int top_bound, int bottom_bound,
                           Graph* graph, Settings* settings) {
    if (vertex->X < left_bound || vertex->X > right_bound ||
            vertex->Y < top_bound || vertex->Y > bottom_bound) return false;

    QString neighbor_vertex_name;
    getVertexNameByCoords(neighbor_x, neighbor_y, neighbor_vertex_name);
    auto itd = graph->find(neighbor_vertex_name.toStdString());
    if (itd != graph->end()) {
        addEdge (vertex, itd->second, 1, *graph, *settings);
        addEdge (itd->second, vertex, 1, *graph, *settings);
    }
}

Graph2dGui::Graph2dGui(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    m_selected_vertex_pnt = new QPoint;
    this->move(801, 0);
}

void Graph2dGui::update(){
    this->repaint();
    QCoreApplication::processEvents();
}

void Graph2dGui::paintEvent(QPaintEvent* pe) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor vertex_color {0xa0, 0xa0, 0xa0};
    painter.setBrush(QBrush(vertex_color));
    QRect indicator_rect;
    for(auto &pair : *m_model->getGraph()) {
        Vertex2d *vertex2d = static_cast<Vertex2d*>(pair.second);
        getVertexBoxInfo(vertex2d->X, vertex2d->Y, &indicator_rect);

        if (pe->region().contains(indicator_rect)) {
            painter.drawEllipse(indicator_rect);
        }
    }
    if (m_selected_vertex_pnt) {
        QRect selector_rect;
        getVertexBoxInfo(m_selected_vertex_pnt->x(), m_selected_vertex_pnt->y(), nullptr, &selector_rect);
        if (pe->region().contains(selector_rect)) {
            painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
            painter.drawRect(selector_rect);
        }
    }
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(3);
    pen.setBrush(Qt::gray);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    QRect proc_mark_rect;
    for (auto &pair2 : *m_search_info->getVisitedVertices())
    {
        getVertexBoxInfo(pair2.second->x(), pair2.second->y(), nullptr, nullptr, &proc_mark_rect);
        painter.drawRect(proc_mark_rect);
    }
    pen.setBrush(Qt::blue);
    painter.setPen(pen);
    for (auto &pair3 : *m_search_info->getProcessedVertices())
    {
        getVertexBoxInfo(pair3.second->x(), pair3.second->y(), nullptr, nullptr, &proc_mark_rect);
        painter.drawRect(proc_mark_rect);
    }

    Vertex2d *source = static_cast<Vertex2d*>(
                m_search_info->getSourceVertex());
    Vertex2d *target = static_cast<Vertex2d*>(
                m_search_info->getTargetVertex());

    if (source && m_state == Graph2dGuiState::SelectSource) {
        pen.setBrush(Qt::green);
        painter.setPen(pen);
        getVertexBoxInfo(source->X, source->Y,
                         nullptr, nullptr, &proc_mark_rect);
        painter.drawRect(proc_mark_rect);
    }

    if (target) {
        pen.setBrush(Qt::red);
        painter.setPen(pen);

        getVertexBoxInfo(target->X, target->Y,
                         nullptr, nullptr, &proc_mark_rect);
        painter.drawRect(proc_mark_rect);
    }

    if (source && m_state == Graph2dGuiState::SelectTarget) {
        pen.setBrush(Qt::green);
        painter.setPen(pen);
        getVertexBoxInfo(source->X, source->Y,
                         nullptr, nullptr, &proc_mark_rect);
        painter.drawRect(proc_mark_rect);
    }

    const vector<Vertex*>* shortest_path = m_search_info->getShortestPath();
    if (shortest_path) {
        pen.setBrush(Qt::yellow);
        painter.setPen(pen);
        for(auto &vertex : *shortest_path) {
            Vertex2d *vertex2d = static_cast<Vertex2d*>(vertex);
            getVertexBoxInfo(vertex2d->X, vertex2d->Y, &indicator_rect);

            if (pe->region().contains(indicator_rect)) {
                painter.drawEllipse(indicator_rect);
            }
        }
    }
}

void Graph2dGui::showEvent(QShowEvent* se) {
    (void)se;
    const QRect rect_geometry = this->geometry();
    int length = VERTEX_SELECTOR_SIDE_LENGTH*VERTEX_X_COUNT + VERTEX_SPACING*(VERTEX_X_COUNT - 1);
    int height = VERTEX_SELECTOR_SIDE_LENGTH*VERTEX_Y_COUNT + VERTEX_SPACING*(VERTEX_Y_COUNT - 1);

    this->setGeometry(rect_geometry.x(), rect_geometry.y(), length + 1, height + 1);
}

void Graph2dGui::mouseMoveEvent(QMouseEvent *event){
    int vertex_x, vertex_y;
    getVertexCoordsByFormCoords(event->x(), event->y(), vertex_x, vertex_y);
    if (vertex_x >= 0 && vertex_x < VERTEX_X_COUNT && vertex_y >= 0 && vertex_y < VERTEX_Y_COUNT) {
        m_selected_vertex_pnt->setX(vertex_x);
        m_selected_vertex_pnt->setY(vertex_y);
    }
    this->repaint();
}

void Graph2dGui::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_search_info->getSearchState() == SearchState::Paused) {
        m_controller->runAlgorithm(AlgoRunMode::Uninterrupted);
        return;
    }
    else if (m_search_info->getSearchState() != SearchState::Stopped) return;

    int coord_x, coord_y;
    getVertexCoordsByFormCoords(event->x(), event->y(), coord_x, coord_y);
    if(coord_x < 0 || coord_x > 8 || coord_y < 0 || coord_y > 8) return;

    if (event->button() == Qt::LeftButton &&
            (m_state == Graph2dGuiState::SelectSource || m_state == Graph2dGuiState::SelectTarget))
    {
        QString str_name;
        getVertexNameByCoords(coord_x, coord_y, str_name);
        Vertex *vertex = m_model->getVertexByName(str_name);
        if (m_state == Graph2dGuiState::SelectSource) {
            m_search_info->setSourceVertex(vertex);
            m_state = Graph2dGuiState::SelectTarget;
        }
        else {
            m_search_info->setTargetVertex(vertex);
            m_state = Graph2dGuiState::SelectSource;
        }
    }
    if (event->button() == Qt::RightButton) {
        Graph* graph = m_model->getGraph();
        Settings* settings = m_model->getSettings();
        QString vertex_name;
        getVertexNameByCoords(coord_x, coord_y, vertex_name);
        auto it = graph->find(vertex_name.toStdString());
        if (it == graph->end()) {
            Vertex2d *new_vertex = new Vertex2d(vertex_name.toStdString(), coord_x, coord_y);
            m_model->getGraph()->insert(make_pair(vertex_name.toStdString(), new_vertex));

            linkToNeighborVertex (new_vertex, coord_x - 1, coord_y, 1, 8, 0, 8, graph, settings);
            linkToNeighborVertex (new_vertex, coord_x + 1, coord_y, 0, 7, 0, 8, graph, settings);
            linkToNeighborVertex (new_vertex, coord_x, coord_y - 1, 0, 8, 1, 8, graph, settings);
            linkToNeighborVertex (new_vertex, coord_x, coord_y + 1, 0, 8, 0, 7, graph, settings);
            m_search_info->clear(false);
        }
        else {
            Vertex* vertex = it->second;
            m_search_info->clear(vertex == m_search_info->getSourceVertex() ||
                                 vertex == m_search_info->getTargetVertex());
            m_model->prepare();
            removeVertex(&vertex, *graph);

        }

        repaint();
    }
}

void Graph2dGui::wheelEvent(QWheelEvent *event) {
    (void) event;
    switch (m_search_info->getSearchState()) {
    case SearchState::Paused:
    case SearchState::Stopped:
        m_controller->runAlgorithm(AlgoRunMode::StepByStep);
        break;
    default:
        break;
    }
}

void Graph2dGui::init(GraphModel *model, GraphController *controller, SearchInfo *search_progress_info) {
    m_search_info = search_progress_info;
    m_search_info->addSubscriber(this);
    m_model = model;
    m_controller = controller;
}
