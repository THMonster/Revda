#include "mainwindow.h"
#include "mpvwidget.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QLabel>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{

    danmakuPlayer = new DanmakuPlayer(this);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(danmakuPlayer);

    setLayout(vl);
    danmakuPlayer->command(QStringList() << "loadfile" << "-");

    QStringList arguments = QCoreApplication::arguments();
    QStringList dmcPy;
    dmcPy.append("-c");
    dmcPy.append("exec(\"\"\"\\nimport time, sys\\nimport threading\\nfrom danmu import DanMuClient\\ndef pp(msg):\\n    print(msg)\\n    sys.stdout.flush()\\ndmc = DanMuClient(sys.argv[1])\\nif not dmc.isValid(): \\n    print('Url not valid')\\n    sys.exit()\\n@dmc.danmu\\ndef danmu_fn(msg):\\n    pp('[%s] %s' % (msg['NickName'], msg['Content']))\\ndmc.start(blockThread = True)\\n\"\"\")");
    dmcPy.append(arguments[2]);
    mProcess = new QProcess(this);
//    connect(mProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readDanmaku);
    mProcess->start("python3", dmcPy);

    readDanmakuTimer = new QTimer(this);
    readDanmakuTimer->start(200);
    connect(readDanmakuTimer, &QTimer::timeout, this, &MainWindow::readDanmaku);
}

MainWindow::~MainWindow()
{
    mProcess->terminate();
    mProcess->waitForFinished(3000);
    mProcess->deleteLater();
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

void MainWindow::readDanmaku()
{
    while(!mProcess->atEnd())
    {
//        QThread::msleep(10);
        QString newDanmaku(mProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(62, ' ');
        if(danmakuPlayer->isDanmakuVisible())
//            disconnect(mProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readDanmaku);
            danmakuPlayer->launchDanmaku(newDanmaku.remove(QRegExp("^\\[.*\\] ")));
    }
}

void MainWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
}
