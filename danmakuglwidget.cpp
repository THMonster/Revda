#include "danmakuglwidget.h"

DanmakuGLWidget::DanmakuGLWidget(QStringList args, QWidget *parent, Qt::WindowFlags f)
        : QOpenGLWidget(parent, f)
{
//    showMaximized();
    setFixedSize(960, 480);
//    setAutoFillBackground(true);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AlwaysStackOnTop);

//    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    danmakuThread = new QThread();
    danmakuLauncher = new DanmakuLauncher(args, this);
//    danmakuLauncher->moveToThread(danmakuThread);
//    connect(danmakuThread, &QThread::finished, danmakuLauncher, &DanmakuLauncher::deleteLater);
//    connect(danmakuLauncher, &DanmakuLauncher::sendDanmaku, this, &DanmakuPlayer::showDanmakuAnimation);
//    connect(danmakuThread, &QThread::started, danmakuLauncher, &DanmakuLauncher::initDmcPy);
//    danmakuThread->start();
}

void DanmakuGLWidget::animate()
{
    update();
}

void DanmakuGLWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    danmakuLauncher->paintDanmaku(&painter, event);
    painter.end();
}
