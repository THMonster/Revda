#include <QCoreApplication>
#include <QtCore>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QProcess>
#include <QStringList>
#include <iostream>

#include "danmakulauncher.h"

bool debug_flag = false;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        if (debug_flag != false) {
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        }
        break;
    case QtInfoMsg:
        std::cerr << msg.toStdString() << std::endl;
        break;
    case QtWarningMsg:
        std::cerr << msg.toStdString() << std::endl;
        break;
    case QtCriticalMsg:
        std::cerr << msg.toStdString() << std::endl;
        break;
    case QtFatalMsg:
        std::cerr << msg.toStdString() << std::endl;
        break;
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QCoreApplication a(argc, argv);
    // QApplication::setApplicationName("QLivePlayer");
    // QApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
//    parser.setApplicationDescription("A cute and useful Live Stream Player with danmaku support.\nProject address: https://github.com/IsoaSFlus/QLivePlayer");
    parser.addHelpOption();
//    parser.addVersionOption();

    QCommandLineOption urlOption(QStringList() << "u" << "url", "The Live Stream url to open", "url", "https://www.douyu.com/2550505");
    parser.addOption(urlOption);
    QCommandLineOption recordOption(QStringList() << "r" << "record", "Record stream to local file", "file", "null");
    parser.addOption(recordOption);
    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Show debug info");
    parser.addOption(debugOption);

    parser.process(a);


    if(QCoreApplication::arguments().size() <= 1)
    {
        qDebug() << "Error: You should at least specify one argument.";
        parser.showHelp();
    }

    if (parser.isSet(debugOption)) {
        debug_flag = true;
    }

    QStringList args;
    args << parser.value(urlOption);
    args << parser.value(recordOption);
    DanmakuLauncher dl(args);
    return a.exec();
}
