#include <QtCore>
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("QLivePlayer");
    QApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();


    parser.process(a);

    MainWindow w;
    w.setWindowTitle("QLivePlayer");
    w.show();
    return a.exec();
}
