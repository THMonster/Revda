#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QProcess>
#include <QStringList>
#include "mainwindow.h"
#include "cliclient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("QLivePlayer");
    QApplication::setApplicationVersion("2.0");

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

    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    setlocale(LC_NUMERIC, "C");

    if(parser.isSet(withoutGUIOption))
    {
        CLIClient cliClient(QStringList() << parser.value(urlOption) << parser.value(streamOption) << parser.value(streamRecordOption) << parser.value(danmakuRecordOption));
        return a.exec();
    }



    MainWindow w(QStringList() << parser.value(urlOption) << parser.value(streamOption) << parser.value(streamRecordOption) << parser.value(danmakuRecordOption));
    w.setWindowTitle("QLivePlayer");
    w.show();
    return a.exec();
}
