#include <QCoreApplication>
#include <QtCore>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QProcess>
#include <QStringList>

#include "danmakulauncher.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // QApplication::setApplicationName("QLivePlayer");
    // QApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
//    parser.setApplicationDescription("A cute and useful Live Stream Player with danmaku support.\nProject address: https://github.com/IsoaSFlus/QLivePlayer");
    parser.addHelpOption();
//    parser.addVersionOption();

    QCommandLineOption urlOption(QStringList() << "u" << "url", "The Live Stream url to open.", "url", "https://www.douyu.com/2550505");
    parser.addOption(urlOption);
//    QCommandLineOption streamOption(QStringList() << "s" << "stream", "The stream to open, default is \"best\".", "stream", "best");
//    parser.addOption(streamOption);
//    QCommandLineOption streamRecordOption(QStringList() << "record-stream", "File to save recorded stream", "filepath", "false");
//    parser.addOption(streamRecordOption);
//    QCommandLineOption danmakuRecordOption(QStringList() << "record-danmaku", "File to save recorded danmaku", "filepath", "false");
//    parser.addOption(danmakuRecordOption);
    QCommandLineOption fifoOption(QStringList() << "p" << "named-pipe", "Named pipe.");
    fifoOption.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(fifoOption);

    parser.process(a);


    if(QCoreApplication::arguments().size() <= 1)
    {
        qDebug() << "Error: You should at least specify one argument.";
        parser.showHelp();
    }


    QStringList args;
    args << parser.value(urlOption);
    DanmakuLauncher dl(args);
    return a.exec();
}
