#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <stack>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTextEdit>
#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include <QWidget>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QPainter>
#include <QRect>
#include <QPaintEvent>
#include <QMessageBox>
#include <QTextStream>
//#include <>
//#include <>
#include "graph.h"
#include "settings.h"
#include "loadsave.h"
#include "algo.h"
#include "informed.h"

map <string, Algorithm> AlgorithmTypes = {
        {"Breadth-first search", BreadthFirstSearch},
        {"Depth-first search", DepthFirstSearch},
        {"Dijkstra", Dijkstra},
        {"Dijkstra2d", Dijkstra2D},
        {"Bidirectional Dijkstra", FastDijkstra},
        {"Bellman-Ford", BellmanFord}
};

void showError (const QString& text) {
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.exec();
    return;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("GraphSearchDemo 1.0");
    QWidget *central_widget = new QWidget();
    this->setCentralWidget(central_widget);
    QVBoxLayout *main_layout = new QVBoxLayout();
    central_widget->setLayout(main_layout);

    QGroupBox *graph_info_gbox = new QGroupBox("Graph info");
    QGroupBox *search_gbox = new QGroupBox("Search");
    main_layout->addWidget(graph_info_gbox);
    main_layout->addWidget(search_gbox);

    QVBoxLayout *graph_info_gb_layout = new QVBoxLayout();
    graph_info_gbox->setLayout(graph_info_gb_layout);

    graph_info_label = new QLabel("Graph not loaded");
    m_graph_text = new QTextEdit();
    m_graph_text->setReadOnly(true);
    graph_info_gb_layout->addWidget(graph_info_label);
    graph_info_gb_layout->addWidget(m_graph_text);

    QGridLayout *search_gb_layout = new QGridLayout();
    search_gbox->setLayout(search_gb_layout);
    QLabel *search_gb_source_label = new QLabel("Source");
    QLabel *search_gb_target_label = new QLabel("Target");
    QLabel *search_gb_algo_label = new QLabel("Algorithm");
    QLabel *search_gb_results_label = new QLabel("Serch results");
    m_search_gb_source_text = new QLineEdit();
    connect(m_search_gb_source_text, SIGNAL(editingFinished()), SLOT(sourceVertexNameEntered()));
    m_search_gb_target_text = new QLineEdit();
    connect(m_search_gb_target_text, SIGNAL(editingFinished()), SLOT(targetVertexNameEntered()));
    m_search_gb_results_text = new QTextEdit();
    m_search_gb_algo_cbox = new QComboBox();
    for (auto &pair : AlgorithmTypes) {
        m_search_gb_algo_cbox->addItem(QString(pair.first.c_str()));
    }
    connect(m_search_gb_algo_cbox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index){ algorithmSelected(index); });

    m_search_gb_run_button = new QPushButton("Run");
    connect(m_search_gb_run_button, SIGNAL(clicked()), SLOT(runAlgo()));

    search_gb_layout->addWidget(search_gb_source_label, 0, 0);
    search_gb_layout->addWidget(search_gb_target_label, 0, 1);
    search_gb_layout->addWidget(search_gb_algo_label, 0, 2);
    search_gb_layout->addWidget(m_search_gb_source_text, 1, 0);
    search_gb_layout->addWidget(m_search_gb_target_text, 1, 1);
    search_gb_layout->addWidget(m_search_gb_algo_cbox, 1, 2);
    search_gb_layout->addWidget(m_search_gb_run_button, 1, 3);
    search_gb_layout->addWidget(search_gb_results_label, 2, 0);
    search_gb_layout->addWidget(m_search_gb_results_text, 3, 0, 1, 4);

    QMenu *fileMenu = menuBar()->addMenu(tr("&Graph"));
    QAction *newAct = new QAction("New");
    QAction *openAct = new QAction("Open");
    QAction *saveAct = new QAction("Save");
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);

    connect(openAct, SIGNAL(triggered()), this, SLOT(openGraph()));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveGraph()));

    m_search_info = new SearchInfo();
    m_search_info->addSubscriber(this);
    m_model = new GraphModel(m_search_info);
    m_controller = GraphController::GetInstance(m_model, m_search_info);

    this->move(0,0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(){
    m_search_gb_source_text->setText(m_search_info->getSourceVertexName());
    m_search_gb_target_text->setText(m_search_info->getTargetVertexName());
    switch (m_search_info->getSearchState()) {
    case SearchState::Running:
    case SearchState::RunningStepByStep:
    case SearchState::Paused:
        m_search_gb_source_text->setEnabled(false);
        m_search_gb_target_text->setEnabled(false);
        m_search_gb_algo_cbox->setEnabled(false);
        m_search_gb_run_button->setEnabled(false);
        break;
    default:
        m_search_gb_source_text->setEnabled(true);
        m_search_gb_target_text->setEnabled(true);
        m_search_gb_algo_cbox->setEnabled(true);
        m_search_gb_run_button->setEnabled(true);
        break;
    }

    this->repaint();
    QCoreApplication::processEvents();
}

void MainWindow::openGraph() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Graph"), ".", tr("Graph Files (*.json *.2d)"));
    if (fileName.isNull()) return;
    int err = m_model->loadFromFile(fileName);
    if (err < 0) {
        showError("Error: Failed loading 2d graph.");
        return;
    }

    QString str_vertex_count = QString::number(m_model->getGraph()->size());
    str_vertex_count.append(" vertices loaded");
    graph_info_label->setText(str_vertex_count);

    if (m_model->getLoadedGraphType() == Graph2D) {
        if (!m_2d_graph_gui) {
            m_2d_graph_gui = new Graph2dGui();
            m_2d_graph_gui->setWindowTitle("2D Graph");
            m_2d_graph_gui->init(m_model, m_controller, m_search_info);
        }
        m_2d_graph_gui->show();
    }
    QString str_graph_text;
    m_model->getTextRepresentation(str_graph_text);
    m_graph_text->setText(str_graph_text);
}

void MainWindow::runAlgo() {
    m_search_gb_results_text->clear();
    if (!m_search_info->getSourceVertex() || !m_search_info->getTargetVertex()) {
        showError("Source or target vertex is not set.");
        return;
    }

    if(m_search_info->getSelectedAlgorithm() == Dijkstra2D && m_model->getLoadedGraphType() != Graph2D) {
        showError("Dijkstra2D only applicable to 2D graphs.");
        return;
    }

    m_controller->runAlgorithm(AlgoRunMode::Uninterrupted);

    if (m_search_info->getLastRunAlgorithm() == BellmanFord &&
            (m_search_info->getResult()->ResultCode == Found ||
             m_search_info->getResult()->ResultCode == NotFound))
    {
        m_search_gb_results_text->append("Paths to all vertices reachable from source: ");
        showPathsToAllVertices();
    }

    switch (m_search_info->getResult()->ResultCode) {
    case NoSourceOrTarget:
        showError("Source or target vertex is absent in the graph.");
        break;
    case SourceIsTarget:
        showError("Source and target are the same vertex.");
        break;
    case NotFound:
        m_search_gb_results_text->append("A path from source to target has not been found.");
        break;
    case NegativeLoopFound:
        m_search_gb_results_text->append("Negative loop was detected and algorithm execution stopped.");
        break;
    case Found:
        m_search_gb_results_text->append("The path from source to target has been found: ");
        showShortestPathInfo();
        break;
    default: return; //Never
    }
}

void MainWindow::showShortestPathInfo() {
        QString path_str;
        QTextStream path_stm(&path_str);
        const vector<Vertex*>* shortest_path = m_search_info->getShortestPath();
        if (!shortest_path) return;
        Vertex* target_vertex = m_search_info->getTargetVertex();

        for(auto &vertex : *shortest_path) {
            path_stm << vertex->Name.c_str();
            if (vertex != target_vertex) path_stm << "->";
        }
        m_search_gb_results_text->append(path_str);

        path_str.clear();
        path_stm << "Shortest path weight: ";
        switch(m_search_info->getLastRunAlgorithm()) {
        case Dijkstra:
        case Dijkstra2D:
        case BellmanFord:
            path_stm << static_cast<DijkstraContext*>(target_vertex->Context)->Weight;
            break;
        case BreadthFirstSearch:
        case DepthFirstSearch:
            path_stm << "unknown (used algorithm is not capable to detect path of lowest weight)";
            break;
        case FastDijkstra: {
            BidirectionalDijkstraResult *fast_dijkstra_result =
                    static_cast<BidirectionalDijkstraResult*>(m_search_info->getResult());
            path_stm << static_cast<BidirectionalDijkstraContext*>(fast_dijkstra_result->ForwardSearchLastVertex->Context)->WeightInForwardSearch +
                static_cast<BidirectionalDijkstraContext*>(fast_dijkstra_result->BackwardSearchLastVertex->Context)->WeightInBackwardSearch +
                fast_dijkstra_result->ConnectingEdgeWeight;
            break;
        }
        default:
            break;
        }
        m_search_gb_results_text->append(path_str);
        if (m_search_info->getLastRunAlgorithm() != BellmanFord) {
            path_str.clear();
            path_stm << "Vertices checked: " << m_search_info->m_checked_count
                << ", processed: " << m_search_info->m_processed_count;
            m_search_gb_results_text->append(path_str);
        }
}

void MainWindow::showPathsToAllVertices() {
    Vertex* source = m_search_info->getSourceVertex();
    Graph &graph = *m_model->getGraph();
    QString path_str;
    QTextStream path_stm(&path_str);
    stack<Vertex*> st;
    for (auto &pair: graph) {
        Vertex* current_vertex = pair.second;
        DijkstraContext* context = static_cast<DijkstraContext*>(current_vertex->Context);
        if (context->Weight == INFINITE_WEIGHT) {
            path_stm << current_vertex->Name.c_str() << ": no path from source to this vertex\n";
            continue;
        }
        Vertex *vertex = current_vertex;
        while (vertex != source) {
            st.push(vertex);
            vertex = static_cast<DijkstraContext*>(vertex->Context)->Parent;
        }
        path_stm << source->Name.c_str();
        while (!st.empty()) {
            path_stm << "->" << st.top()->Name.c_str();
            st.pop();
        }
        path_stm << " (weight: " << static_cast<DijkstraContext*>(current_vertex->Context)->Weight  << ")";
        m_search_gb_results_text->append(path_str);
        path_str.clear();
    }
}

void MainWindow::sourceVertexNameEntered()
{
    Vertex *vertex = findVertex(m_search_gb_source_text->text().toStdString().c_str(),
        *m_model->getGraph());
    if (vertex) {
        m_search_info->setSourceVertex(vertex);
    }
}

void MainWindow::targetVertexNameEntered()
{
    Vertex *vertex = findVertex(m_search_gb_target_text->text().toStdString().c_str(),
        *m_model->getGraph());
    if (vertex) {
        m_search_info->setTargetVertex(vertex);
    }
}

void MainWindow::algorithmSelected(int index)
{
    string str_algo{m_search_gb_algo_cbox->itemText(m_search_gb_algo_cbox->currentIndex()).toStdString()};
    m_search_info->setSelectedAlgorithm(AlgorithmTypes[str_algo]);
}

void MainWindow::saveGraph()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Graph"), ".", tr("Graph Files (*.json *.2d)"));
    if (fileName.isNull()) return;
    int err = m_model->saveToFile(fileName);
    if (err < 0) {
        showError("Error: Failed saving graph.");
        return;
    }

    QString str_vertex_count = QString::number(m_model->getGraph()->size());
    str_vertex_count.append(" vertices saved");
    graph_info_label->setText(str_vertex_count);

    QString str_graph_text;
    m_model->getTextRepresentation(str_graph_text);
    m_graph_text->setText(str_graph_text);
}
