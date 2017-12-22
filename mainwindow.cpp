#include "mainwindow.h"
#include "mpvwidget.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QLabel>
#include <QFile>
#include <QUuid>

MainWindow::MainWindow(QStringList args,QWidget *parent) : QWidget(parent)
{
    this->args = args;
    startStreamlinkProcess();
//    qDebug() << args;
    danmakuPlayer = new DanmakuPlayer(args, this, 0);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(danmakuPlayer);

    setLayout(vl);
    danmakuPlayer->command(QStringList() << "loadfile" << namedPipe);

    connect(danmakuPlayer, &DanmakuPlayer::refreshStream, this, &MainWindow::refreshDanmakuPlayer);

//    qDebug() << QString("main thread id:") << QThread::currentThreadId();
}

MainWindow::~MainWindow()
{
    QProcess::execute("rm " + namedPipe);
    streamLinkProcess->terminate();
    streamLinkProcess->waitForFinished(3000);
    streamLinkProcess->deleteLater();
}

void MainWindow::startStreamlinkProcess()
{
    namedPipe = "/tmp/qlivesplayer-" + QUuid::createUuid().toString();
    QProcess::execute("mkfifo " + namedPipe);
    streamLinkProcess = new QProcess(this);
    if(args.at(2) == QString("false")) {
        streamLinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O > " + namedPipe + "\"");
    } else
        streamLinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O | tee " + args.at(2) + QString(".%1").arg(videoPart++) + " > " + namedPipe + "\"");
    if(!streamLinkProcess->waitForStarted())
        QApplication::exit(1);
}

void MainWindow::refreshDanmakuPlayer()
{
    danmakuPlayer->command(QStringList() << "stop");
    streamLinkProcess->terminate();
    streamLinkProcess->waitForFinished(3000);
    delete streamLinkProcess;
    QProcess::execute("rm " + namedPipe);
    QProcess::execute("mkfifo " + namedPipe);
    streamLinkProcess = new QProcess(this);
    if(args.at(2) == QString("false")) {
        streamLinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O > " + namedPipe + "\"");
    } else
        streamLinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O | tee " + args.at(2) + QString(".%1").arg(videoPart++) + " > " + namedPipe + "\"");
    if(!streamLinkProcess->waitForStarted())
        QApplication::exit(1);
    danmakuPlayer->command(QStringList() << "loadfile" << namedPipe);
}

void MainWindow::openMedia()
{
    QString file = QFileDialog::getOpenFileName(0, "Open a video");
    if (file.isEmpty())
        return;
//    m_mpv->command(QStringList() << "loadfile" << file);
    danmakuPlayer->command(QStringList() << "loadfile" << "-");
}

void MainWindow::seek(int pos)
{
    danmakuPlayer->command(QVariantList() << "seek" << pos << "absolute");
}

void MainWindow::pauseResume()
{
    const bool paused = danmakuPlayer->getProperty("pause").toBool();
    danmakuPlayer->setProperty("pause", !paused);
}

void MainWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
}
