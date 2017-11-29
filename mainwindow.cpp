#include "mainwindow.h"
#include "mpvwidget.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QLabel>
#include <QFile>

MainWindow::MainWindow(QStringList args,QWidget *parent) : QWidget(parent)
{
    this->args = args;
    startStreamlinkProcess();
    qDebug() << args;
    danmakuPlayer = new DanmakuPlayer(args, this, 0);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(danmakuPlayer);

    setLayout(vl);
    danmakuPlayer->command(QStringList() << "loadfile" << namedPipe);

    QStringList dmcPy;
    dmcPy.append("-c");
    dmcPy.append("exec(\"\"\"\\nimport time, sys\\nimport threading\\nfrom danmu import DanMuClient\\ndef pp(msg):\\n    print(msg)\\n    sys.stdout.flush()\\ndmc = DanMuClient(sys.argv[1])\\nif not dmc.isValid(): \\n    print('Url not valid')\\n    sys.exit()\\n@dmc.danmu\\ndef danmu_fn(msg):\\n    pp('[%s] %s' % (msg['NickName'], msg['Content']))\\ndmc.start(blockThread = True)\\n\"\"\")");
    dmcPy.append(args.at(0));
    dmcPyProcess = new QProcess(this);
//    connect(mProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readDanmaku);
    dmcPyProcess->start("python3", dmcPy);

    readDanmakuTimer = new QTimer(this);
    readDanmakuTimer->start(200);
//    mThread = new QThread(this);
//    qDebug() << QString("main thread id:") << QThread::currentThreadId();

//    connect(mThread, &QThread::started, this, &MainWindow::startThread);
    connect(readDanmakuTimer, &QTimer::timeout, this, &MainWindow::readDanmaku);
}

MainWindow::~MainWindow()
{
    QProcess::execute("rm " + namedPipe);
    dmcPyProcess->terminate();
    dmcPyProcess->waitForFinished(3000);
    dmcPyProcess->deleteLater();
    streamLinkProcess->terminate();
    streamLinkProcess->waitForFinished(3000);
    streamLinkProcess->deleteLater();
}

void MainWindow::startStreamlinkProcess()
{
    namedPipe = "/tmp/qlivesplayersdfadsfsdewe";
    QProcess::execute("mkfifo " + namedPipe);
    streamLinkProcess = new QProcess(this);
    if(args.at(2) == QString("false")) {
        streamLinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O > " + namedPipe + "\"");
    } else
        streamLinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O | tee " + args.at(2) + " > " + namedPipe + "\"");
    if(!streamLinkProcess->waitForStarted())
        QApplication::exit(1);
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

void MainWindow::startThread()
{
//    emit mThread->start();
}

void MainWindow::readDanmaku()
{
    while(!dmcPyProcess->atEnd())
    {
//        QThread::msleep(10);
        QString newDanmaku(dmcPyProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(62, ' ');
        if(danmakuPlayer->isDanmakuVisible())
            danmakuPlayer->launchDanmaku(newDanmaku.remove(QRegExp("^\\[.*\\] ")));
    }
}

void MainWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
}
