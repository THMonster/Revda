#include "danmakuglwidget.h"

DanmakuGLWidget::DanmakuGLWidget(QStringList args, QWidget *parent)
    : QOpenGLWidget(parent)
{
    setAttribute(Qt::WA_AlwaysStackOnTop);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
//    setAttribute(Qt::WA_NoSystemBackground, false);
//    setStyleSheet("background-color: rgba(111,111,111,255)");
//    QSurfaceFormat fmt;
//    fmt.setAlphaBufferSize(8);
//    setFormat(fmt);

    danmakuThread = new QThread();
    danmakuLauncher = new DanmakuLauncher(args, this);
    danmakuLauncher->moveToThread(danmakuThread);
    connect(danmakuThread, &QThread::finished, danmakuLauncher, &DanmakuLauncher::deleteLater);
    connect(danmakuThread, &QThread::started, danmakuLauncher, &DanmakuLauncher::initDL);
    danmakuThread->start();

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    updateTimer->start(16);
}


DanmakuGLWidget::~DanmakuGLWidget()
{
    danmakuThread->quit();
    danmakuLauncher->deleteLater();
}


void DanmakuGLWidget::paintGL()
{
//    QOpenGLFramebufferObject::bindDefault();
//    QOpenGLPaintDevice fboPaintDev(width(), height());
//    QPainter painter(&fboPaintDev);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
//    painter.setCompositionMode (QPainter::CompositionMode_Source);
//    painter.fillRect(this->rect(), Qt::transparent);
//    painter.setCompositionMode (QPainter::CompositionMode_SourceOver);
    danmakuLauncher->paintDanmaku(&painter);


//        painter.setCompositionMode(QPainter::CompositionMode_Source);
//        painter.fillRect(event->rect(), Qt::transparent);
//        painter.fillRect(0,0,400,400,Qt::white);

//        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//        painter.drawText(200, 400, "TEST");
    painter.end();
}

void DanmakuGLWidget::setStreamReadyFlag(bool flag)
{
    danmakuLauncher->setStreamReadyFlag(flag);
}
