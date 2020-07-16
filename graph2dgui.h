#ifndef GRAPH2DGUI_H
#define GRAPH2DGUI_H

#include <QWidget>
#include "graph.h"
#include "searchinfo.h"
#include "graphmodel.h"
#include "graphcontroller.h"

enum class Graph2dGuiState {
    DisplayOnly,
    SelectSource,
    SelectTarget,
};

class Graph2dGui : public QWidget, public ISubscriber
{
    Q_OBJECT
private:
    QPoint *m_selected_vertex_pnt;
    SearchInfo *m_search_info = nullptr;
    GraphModel *m_model = nullptr;
    GraphController *m_controller = nullptr;
    Graph2dGuiState m_state = Graph2dGuiState::SelectSource;
protected:
    void paintEvent(QPaintEvent* pe) override;
    void showEvent(QShowEvent* se) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
public:
    explicit Graph2dGui(QWidget *parent = nullptr);
    void init(GraphModel *model, GraphController *controller, SearchInfo *search_progress_info);

    virtual void update() override;

signals:

};

#endif // GRAPH2DGUI_H
