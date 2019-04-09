#include "cliclient.h"

CLIClient::CLIClient(QStringList args)
    : QObject()
{
//    qDebug() << "CLIClient::CLIClient";
    this->args = args;

    qDebug() << "Waiting for stream...";
    checkProcess = new QProcess(this);
    checkProcess->start("bash -c \"streamlink " + args.at(0) + "\"");
    connect(checkProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CLIClient::checkStreamAvailable);

    danmakuLauncherThread = new QThread();
    danmakuLauncher = new DanmakuLauncher(args, nullptr, 1280, 720);


    checkStreamReadyTimer = new QTimer(this);
    connect(checkStreamReadyTimer, &QTimer::timeout, this, &CLIClient::checkStreamReady);

    time.start();

    paintTimer = new QTimer(this);
    connect(paintTimer, &QTimer::timeout, danmakuLauncher, &DanmakuLauncher::paintDanmakuCLI);

}

CLIClient::~CLIClient()
{
//    qDebug() << "CLIClient::~CLIClient";
    QProcess::execute("rm " + namedPipe);
    danmakuLauncherThread->quit();
    danmakuLauncher->deleteLater();
    streamlinkProcess->terminate();
    streamlinkProcess->waitForFinished(3000);
    streamlinkProcess->deleteLater();
}


void CLIClient::checkStreamReady()
{
//    qDebug() << "CLIClient::checkStreamReady";

    if (streamReady == true && args.at(3) != "false") {
        checkStreamReadyTimer->stop();
        danmakuLauncher->setPlayingState(0);
    }
}


void CLIClient::startStreamlinkProcess()
{
//    qDebug() << "CLIClient::startStreamlinkProcess";
    if(namedPipe == QString(""))
    {
        namedPipe = "/tmp/qlivesplayer-" + QUuid::createUuid().toString();
        QProcess::execute("mkfifo " + namedPipe);
        streamlinkProcess = new QProcess(this);
        connect(streamlinkProcess, &QProcess::readyReadStandardError, this, &CLIClient::onStreamlinkStderrReady);
        connect(streamlinkProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CLIClient::onStreamlinkFinished);
    }
    if(streamAvailable)
    {
        if(args.at(2) == QString("false")) {
            streamlinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O > " + namedPipe + "\"");
        } else {
            streamlinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O | tee " + args.at(2) + QString(".%1").arg(videoPart++) + " > " + "/dev/null" + "\"");
        }
        if(!streamlinkProcess->waitForStarted()) {
//            QApplication::exit(1);
        }

        if (boardcastStarted == false) {
            boardcastStarted = true;
            startDanmakuLauncher();
        }
    }
}

void CLIClient::onStreamlinkFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
//    qDebug() << "CLIClient::onStreamlinkFinished";
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    streamAvailable = false;
    danmakuLauncher->setStreamReadyFlag(false);
    streamReady = false;
    danmakuLauncher->setPlayingState(1);
    qDebug().noquote() << "Streamlink quited, now try to restart...";
    checkProcess->start("bash -c \"streamlink " + args.at(0) + "\"");

}

void CLIClient::checkStreamAvailable()
{
//    qDebug() << "CLIClient::checkStreamAvailable";
    QThread::msleep(500);
    QString stdinfo(checkProcess->readAllStandardOutput());
//    qDebug() << stdinfo;
    if(stdinfo.contains(QRegExp("Available streams")))
    {
        streamAvailable = true;
        QThread::msleep(500);
        startStreamlinkProcess();
        checkStreamReadyTimer->start(100);

    }
    else
    {
        streamAvailable = false;
        checkProcess->start("bash -c \"streamlink " + args.at(0) + "\"");
    }
}

void CLIClient::startDanmakuLauncher()
{
    danmakuLauncher->moveToThread(danmakuLauncherThread);
    connect(danmakuLauncherThread, &QThread::finished, danmakuLauncher, &DanmakuLauncher::deleteLater);

    connect(danmakuLauncherThread, &QThread::started, danmakuLauncher, &DanmakuLauncher::initDL);
    danmakuLauncherThread->start();
}

void CLIClient::onStreamlinkStderrReady()
{
//    qDebug() << "CLIClient::onStreamlinkStderrReady";
    QString stdinfo(streamlinkProcess->readAllStandardError());
    stdinfo.chop(1);
    if(stdinfo.contains(QRegExp("Opening stream:"))) {
        streamReady = true;
        paintTimer->start(16);
        danmakuLauncher->setStreamReadyFlag(true);
    }
    qDebug().noquote() << stdinfo;
}
