#include <QtCore>
#include <QApplication>
#include "launcher.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("QLivePlayer");
    QApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();


    parser.process(a);

    Launcher launcher;
    launcher.start();
    return a.exec();
}
