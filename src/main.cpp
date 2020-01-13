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

    QCommandLineOption hostOption(QStringList() << "i" << "host", "Host's ip", "ip", "127.0.0.1");
    parser.addOption(hostOption);
    QCommandLineOption portOption(QStringList() << "p" << "port", "Host's port", "port", "6600");
    parser.addOption(portOption);
    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Show debug info");
    parser.addOption(debugOption);

    parser.process(a);

    MainWindow w;
    w.setWindowTitle("QLivePlayer");
    w.show();
    return a.exec();
}
