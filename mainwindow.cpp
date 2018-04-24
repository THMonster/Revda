#include "mainwindow.h"
#include "danmakuplayer.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QLabel>
#include <QFile>
#include <QUuid>

MainWindow::MainWindow(QStringList args,QWidget *parent) : QMainWindow(parent)
{
    this->args = args;
    startStreamlinkProcess();

//    setAttribute(Qt::WA_TranslucentBackground);
//    setWindowFlags(Qt::FramelessWindowHint);
    danmakuPlayer = new DanmakuPlayer(args, this, 0);
////    mpvEmWidget = new MpvEmWidget(this);

//    QVBoxLayout *vl = new QVBoxLayout(this);
    setCentralWidget(danmakuPlayer);
//    QLabel *ql = new QLabel(this);
//    ql->setPixmap(QPixmap("/home/midorikawa/Pictures/Screenshots/屏幕截图_1.png"));
//    vl->setContentsMargins(0,0,0,0);
//    vl->addWidget(danmakuPlayer);
//    setLayout(vl);

    danmakuGLWidget = new DanmakuGLWidget(args, this);
//        QVBoxLayout *vl = new QVBoxLayout(this);
//        vl->setContentsMargins(0,0,0,0);
//        vl->addWidget(danmakuGLWidget);
//        setLayout(vl);

    danmakuGLWidget->resize(720, 720);
    danmakuGLWidget->show();
//    QPushButton *qpb = new QPushButton(this);
//    qpb->setAttribute(Qt::WA_AlwaysStackOnTop);
//    qpb->setAttribute(Qt::WA_TranslucentBackground);
//    qpb->setStyleSheet("background-color: rgba(111,111,111,0)");
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
