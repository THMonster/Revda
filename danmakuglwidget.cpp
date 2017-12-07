#include "danmakuglwidget.h"

DanmakuGLWidget::DanmakuGLWidget(QStringList args, QWidget *parent, Qt::WindowFlags f)
        : QOpenGLWidget(parent, f)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    danmakuThread = new QThread();
    danmakuLauncher = new DanmakuLauncher(args, this);
    danmakuLauncher->moveToThread(danmakuThread);
    connect(danmakuThread, &QThread::finished, danmakuLauncher, &DanmakuLauncher::deleteLater);
    connect(danmakuThread, &QThread::started, danmakuLauncher, &DanmakuLauncher::initDL);
    danmakuThread->start();
    paintTimer = new QTimer(this);
    connect(paintTimer, &QTimer::timeout, this, &DanmakuGLWidget::animate);
//    paintTimer->start(16);
}

DanmakuGLWidget::~DanmakuGLWidget()
{
    danmakuThread->quit();
    danmakuLauncher->deleteLater();
}

void DanmakuGLWidget::animate()
{
    update();
}

//void DanmakuGLWidget::paintEvent(QPaintEvent *event)
//{
//    QPainter painter;
//    painter.begin(this);
//    painter.setRenderHint(QPainter::TextAntialiasing);
//    danmakuLauncher->paintDanmaku(&painter, event);
//    painter.end();
//}
