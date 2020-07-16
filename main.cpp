#include "mainwindow.h"
#include <QApplication>

QString __version__ = "1.0.0.8";

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("./");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
