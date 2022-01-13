#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QProcess>
#include <QStringList>
#include <QtCore>
#include <initializer_list>
#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "bilivideo.h"
#include "qlphelper.h"

bool debug_flag = false;

void
ignoreUnixSignals(std::initializer_list<int> ignoreSignals)
{
    // all these signals will be ignored.
    for (int sig : ignoreSignals)
        signal(sig, SIG_IGN);
}

void
catchUnixSignals(std::initializer_list<int> quitSignals)
{
    auto handler = [](int sig) -> void {
        // blocking and not aysnc-signal-safe func are valid
        //        printf("\nquit the application by signal(%d).\n", sig);
        QCoreApplication::quit();
    };

    sigset_t blocking_mask;
    sigemptyset(&blocking_mask);
    for (auto sig : quitSignals)
        sigaddset(&blocking_mask, sig);

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_mask = blocking_mask;
    sa.sa_flags = 0;

    for (auto sig : quitSignals)
        sigaction(sig, &sa, nullptr);
}

void
myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
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

int
main(int argc, char* argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QCoreApplication a(argc, argv);
    catchUnixSignals({ SIGQUIT, SIGINT, SIGTERM, SIGHUP });
    QCoreApplication::setApplicationName("QLivePlayer");
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(
      "A cute and useful Live Stream Player with danmaku support.\nProject address: https://github.com/IsoaSFlus/QLivePlayer");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption urlOption(QStringList() << "u"
                                               << "url",
                                 "The url to open", "url", "null");
    parser.addOption(urlOption);
    QCommandLineOption recordOption(QStringList() << "r"
                                                  << "record",
                                    "Record stream to local file", "file", "null");
    parser.addOption(recordOption);
    QCommandLineOption strictStreamOption(QStringList() << "strict-stream", "Useful for preventing non-monotonous DTS problem");
    parser.addOption(strictStreamOption);
    QCommandLineOption debugOption(QStringList() << "d"
                                                 << "debug",
                                   "Show debug info");
    parser.addOption(debugOption);
    QCommandLineOption fontScaleOption(QStringList() << "fs", "Set font scale", "float", "null");
    parser.addOption(fontScaleOption);
    QCommandLineOption fontAlphaOption(QStringList() << "fa", "Set font alpha", "float", "null");
    parser.addOption(fontAlphaOption);
    QCommandLineOption quietOption(QStringList() << "quiet", "Don't print danmaku to terminal");
    parser.addOption(quietOption);

    parser.process(a);

    if (QCoreApplication::arguments().size() <= 1) {
        qDebug() << "Error: You should at least specify one argument.";
        parser.showHelp();
    }

    auto url = parser.value(urlOption);
    if (url.startsWith("qliveplayer://")) {
        url.remove(0, 14);
        if (!url.startsWith("http")) {
            url = QStringLiteral("https://") + url;
        }
    }

    if (parser.isSet(debugOption)) {
        debug_flag = true;
    }

    if (url.contains("bilibili.com/video") || url.contains("bilibili.com/bangumi")) {
        QStringList args;
        args << parser.value(fontScaleOption);
        args << parser.value(fontAlphaOption);
        auto bv = new BV::BiliVideo(args, &a);
        if (parser.value(recordOption) != "null") {
            bv->setSavedFilePath(parser.value(recordOption));
        }
        bv->run(url);
    } else {
        QStringList args;
        args << url;
        args << parser.value(recordOption);
        args << (parser.isSet(strictStreamOption) ? "true" : "false");
        args << (parser.isSet(debugOption) ? "true" : "false");
        args << parser.value(fontScaleOption);
        args << parser.value(fontAlphaOption);
        args << (parser.isSet(quietOption) ? "true" : "false");
        auto qlphelper = new QLPHelper(args, &a);
        qlphelper->start();
    }
    return a.exec();
}
