#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QProcess>
#include "mainwindow.h"
#include "clirecorder.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("QLivePlayer");
    QApplication::setApplicationVersion("1.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("A cute and useful Live Stream Player with danmaku support.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption urlOption(QStringList() << "u" << "url", "The Live Stream url to open.", "url", "https://www.douyu.com/2550505");
    parser.addOption(urlOption);
    QCommandLineOption streamOption(QStringList() << "s" << "stream", "The stream to open, default is \"best\".", "stream", "best");
    parser.addOption(streamOption);
    QCommandLineOption streamRecordOption(QStringList() << "record-stream", "File to save recorded stream", "filepath", "false");
    parser.addOption(streamRecordOption);
    QCommandLineOption danmakuRecordOption(QStringList() << "record-danmaku", "File to save recorded danmaku", "filepath", "false");
    parser.addOption(danmakuRecordOption);
    QCommandLineOption withoutGUIOption(QStringList() << "c" << "without-gui", "CLI mode, available only with recording.");
    parser.addOption(withoutGUIOption);

    parser.process(a);


    if(QCoreApplication::arguments().size() <= 1)
    {
        qDebug().noquote() << "Error: You should at least specify one argument.";
        parser.showHelp();
        exit(1);
    }

    if(QCoreApplication::arguments().at(1) != "bypass-parser")
    {
        if(parser.value(streamRecordOption) != "false")
        {
            QProcess::execute("bash -c \"streamlink " + parser.value(urlOption) + " " + parser.value(streamOption) + " -O | tee '" + parser.value(streamRecordOption) + "' | " + QCoreApplication::applicationFilePath() + " bypass-parser " + parser.value(urlOption) + " '" + parser.value(danmakuRecordOption) + "' " + QString(parser.isSet(withoutGUIOption) ? "true" : "false") + "\"");
            exit(0);
        }
        else
        {
            QProcess::execute("bash -c \"streamlink " + parser.value(urlOption) + " " + parser.value(streamOption) + " -O | " + QCoreApplication::applicationFilePath() + " bypass-parser " + parser.value(urlOption) + " '" + parser.value(danmakuRecordOption) + "' " + QString(parser.isSet(withoutGUIOption) ? "true" : "false") + "\"");
            exit(0);
        }
    }

    setlocale(LC_NUMERIC, "C");

    if(QCoreApplication::arguments().at(4) == "true")
    {
//        QCoreApplication b(argc, argv);
        CLIRecorder cliRecorder;
        return a.exec();
    }

    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.

    MainWindow w;
    w.setWindowTitle("QLivePlayer");
    w.show();
    return a.exec();
}
