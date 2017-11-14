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
//    m_slider = new QSlider();

//    m_slider->setOrientation(Qt::Horizontal);
//    m_openBtn = new QPushButton("Open");
//    m_playBtn = new QPushButton("Pause");
//    QHBoxLayout *hb = new QHBoxLayout();
//    hb->addWidget(m_openBtn);
//    hb->addWidget(m_playBtn);
    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(danmakuPlayer);
//    vl->addWidget(m_slider);
//    vl->addLayout(hb);
    setLayout(vl);
    danmakuPlayer->command(QStringList() << "loadfile" << "-");
//    connect(m_slider, SIGNAL(sliderMoved(int)), SLOT(seek(int)));
//    connect(m_openBtn, SIGNAL(clicked()), SLOT(openMedia()));
//    connect(m_playBtn, SIGNAL(clicked()), SLOT(pauseResume()));
//    connect(m_mpv, SIGNAL(positionChanged(int)), m_slider, SLOT(setValue(int)));
//    connect(m_mpv, SIGNAL(durationChanged(int)), this, SLOT(setSliderRange(int)));

    QStringList arguments = QCoreApplication::arguments();
    QStringList dmcPy;
    dmcPy.append("-c");
    dmcPy.append("exec(\"\"\"\\nimport time, sys\\nimport threading\\nfrom danmu import DanMuClient\\ndef pp(msg):\\n    print(msg)\\n    sys.stdout.flush()\\ndmc = DanMuClient(sys.argv[1])\\nif not dmc.isValid(): \\n    print('Url not valid')\\n    sys.exit()\\n@dmc.danmu\\ndef danmu_fn(msg):\\n    pp('[%s] %s' % (msg['NickName'], msg['Content']))\\ndmc.start(blockThread = True)\\n\"\"\")");
    dmcPy.append(arguments[1]);
    mProcess = new QProcess(this);
    connect(mProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readDanmaku);
    mProcess->start("python3", dmcPy);
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
        QString newDanmaku(mProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(55, ' ');
        if(danmakuPlayer->isDanmakuVisible())
            danmakuPlayer->launchDanmaku(newDanmaku.remove(QRegExp("^\\[.*\\] ")));
    }
}

void MainWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
}
