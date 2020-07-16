#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <QMainWindow>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include "QComboBox"
#include "graph2dgui.h"
#include "searchinfo.h"
#include "graphmodel.h"
#include "graphcontroller.h"
//#include ""
//#include ""

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public ISubscriber
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    virtual void update() override;

private:
    Ui::MainWindow *ui;
    QLabel *graph_info_label;
    QTextEdit *m_graph_text;
    QLineEdit *m_search_gb_source_text;
    QLineEdit *m_search_gb_target_text;
    QTextEdit *m_search_gb_results_text;
    QPushButton *m_search_gb_run_button;
    QComboBox *m_search_gb_algo_cbox;
    Graph2dGui *m_2d_graph_gui = nullptr;
    GraphModel *m_model = nullptr;
    GraphController *m_controller = nullptr;
    SearchInfo *m_search_info = nullptr;
    void showPathsToAllVertices();
    void showShortestPathInfo();
private slots:
    void openGraph();
    void runAlgo();
    void saveGraph();
    void sourceVertexNameEntered();
    void targetVertexNameEntered();
    void algorithmSelected(int index);
};

#endif // MAINWINDOW_H
