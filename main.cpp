#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QProcess>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("QLivePlayer");
    QApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("A cute and useful Live Stream Player with danmaku support.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption urlOption(QStringList() << "u" << "url", "The Live Stream url to open.", "url", "https://www.douyu.com/2550505");
    parser.addOption(urlOption);
    QCommandLineOption streamOption(QStringList() << "s" << "stream", "The stream to open, default is \"best\".", "stream", "best");
    parser.addOption(streamOption);

    parser.process(a);

    if(QCoreApplication::arguments().size() <= 1)
    {
        qDebug().noquote() << "Error: You should at least specify one argument.";
        parser.showHelp();
        exit(1);
    }

    if(QCoreApplication::arguments().at(1) != "bypass-parser")
    {
        QProcess::startDetached("bash -c \"streamlink " + parser.value(urlOption) + " " + parser.value(streamOption) + " -O | " + QCoreApplication::applicationFilePath() + " bypass-parser " + parser.value(urlOption) + "\"");
        exit(0);
    }


    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    setlocale(LC_NUMERIC, "C");
    MainWindow w;
    w.setWindowTitle("QLivePlayer");
    w.show();
    return a.exec();
}
