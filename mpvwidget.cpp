#include "mpvwidget.h"
#include <stdexcept>
#include <QtGui/QOpenGLContext>
#include <QtCore/QMetaObject>
#include <QFontMetrics>
#include <QPainterPath>

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return NULL;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

MpvWidget::MpvWidget(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "msg-level", "all=v");
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Make use of the MPV_SUB_API_OPENGL_CB API.
    mpv::qt::set_option_variant(mpv, "vo", "opengl-cb");

    // Request hw decoding, just for testing.
    mpv::qt::set_option_variant(mpv, "hwdec", "auto");

    mpv_gl = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!mpv_gl)
        throw std::runtime_error("OpenGL not compiled in");
    mpv_opengl_cb_set_update_callback(mpv_gl, MpvWidget::on_update, (void *)this);
    connect(this, SIGNAL(frameSwapped()), SLOT(swapped()));

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(mpv, wakeup, this);

    initDanmaku();
    initLoadDanmakuTimer();
    initDensityTimer();
}



MpvWidget::~MpvWidget()
{
    makeCurrent();
    if (mpv_gl)
        mpv_opengl_cb_set_update_callback(mpv_gl, NULL, NULL);
    // Until this call is done, we need to make sure the player remains
    // alive. This is done implicitly with the mpv::qt::Handle instance
    // in this class.
    mpv_opengl_cb_uninit_gl(mpv_gl);
}

void MpvWidget::command(const QVariant& params)
{
    mpv::qt::command_variant(mpv, params);
}

void MpvWidget::setProperty(const QString& name, const QVariant& value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant MpvWidget::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

void MpvWidget::addNewDanmaku(QString danmaku)
{
    danmakuPool[writeDanmakuIndex++] = "0" + danmaku; //0为未读
    writeDanmakuIndex = writeDanmakuIndex % 20;
}

void MpvWidget::initDanmaku()
{
    writeDanmakuIndex = 0;
    readDanmakuIndex = 0;
    timeStamp = "";

    int i;
    for(i = 0; i < 20; i++)
    {
        danmakuPool << "NULL";
    }
}

void MpvWidget::initDensityTimer()
{
    danmakuDensityTimer = new QTimer(this);
    connect(danmakuDensityTimer, &QTimer::timeout, this, &MpvWidget::launchDanmaku);
    danmakuDensityTimer->start(100);
}

void MpvWidget::initLoadDanmakuTimer()
{
    loadDanmakuTimer = new QTimer(this);
    connect(loadDanmakuTimer, &QTimer::timeout, this, &MpvWidget::loadDanmaku);
    loadDanmakuTimer->start(500);
}

void MpvWidget::launchDanmaku()
{
    if((danmakuPool[readDanmakuIndex] != "NULL") && (danmakuPool[readDanmakuIndex].at(0) != "1"))
    {
//        qDebug() << "launch";

        int danmakuPos = ((qrand()%10) * (this->height() / 10));
        int danmakuSpeed = this->width() * 11;

        QLabel* danmaku;
        danmaku = new QLabel(this);
        QPalette mPalette;
        mPalette.setColor(QPalette::WindowText,Qt::white);
        danmaku->setPalette(mPalette);
        danmakuPool[readDanmakuIndex].remove(0, 1);
        danmaku->setText(danmakuPool[readDanmakuIndex]);
        danmakuPool[readDanmakuIndex++].insert(0, "1");
        readDanmakuIndex = readDanmakuIndex % 20;

        danmaku->setFont(QFont("思源黑体 CN", 12, QFont::Bold));
        QPropertyAnimation* mAnimation=new QPropertyAnimation(danmaku, "pos");
        mAnimation->setStartValue(QPoint(-80, danmakuPos));
        mAnimation->setEndValue(QPoint(this->width(), danmakuPos));
        mAnimation->setDuration(danmakuSpeed);
        mAnimation->setEasingCurve(QEasingCurve::Linear);
        danmaku->show();
        mAnimation->start();

        connect(mAnimation, &QPropertyAnimation::finished, danmaku, &QLabel::deleteLater);
    }else
    {
        readDanmakuIndex++;
        readDanmakuIndex = readDanmakuIndex % 20;
        return;
    }
}

void MpvWidget::loadDanmaku()
{
//    qDebug() << "load";
    QFile file("/tmp/danmaku.temp");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString newTimeStamp(in.readLine());
        if(timeStamp != newTimeStamp)
        {
            timeStamp = newTimeStamp;
            while(!in.atEnd())
            {
                addNewDanmaku(in.readLine());
            }
        }
        file.close();
    }
}

void MpvWidget::initializeGL()
{
    int r = mpv_opengl_cb_init_gl(mpv_gl, NULL, get_proc_address, NULL);
    if (r < 0)
        throw std::runtime_error("could not initialize OpenGL");
}

void MpvWidget::paintGL()
{
    mpv_opengl_cb_draw(mpv_gl, defaultFramebufferObject(), width(), -height());
}

void MpvWidget::swapped()
{
    mpv_opengl_cb_report_flip(mpv_gl, 0);
}

void MpvWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void MpvWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT durationChanged(time);
            }
        }
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}

// Make Qt invoke mpv_opengl_cb_draw() to draw a new/updated video frame.
void MpvWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's opengl-cb API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        swapped();
        doneCurrent();
    } else {
        update();
    }
}

void MpvWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "maybeUpdate");
}
