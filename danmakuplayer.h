#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include "danmakurecorder.h"
#include "danmakulauncher.h"
#include "danmakuglwidget.h"
#include "mpvemwidget.h"
#include <QtWidgets/QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>
#include <QPainter>
#include <QFontMetrics>
#include <QFont>
#include <QLabel>
#include <QPalette>
#include <QPropertyAnimation>
#include <QList>
#include <QStringList>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QtGlobal>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <cmath>
#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>
#include <QMainWindow>
#include <QCoreApplication>
#include <QApplication>
#include <QtGui>
#include <QElapsedTimer>
#include <QVBoxLayout>
class DanmakuGLWidget;
class DanmakuLauncher;
class MpvWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    MpvWidget(QWidget *parent = 0, Qt::WindowFlags f = 0, bool cli = false);
    ~MpvWidget();
    void command(const QVariant& params);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;
//    QSize sizeHint() const { return QSize(1280, 720);}

Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    DanmakuLauncher* danmakuLauncher = 0;
    QTimer* updateTimer;
private Q_SLOTS:
    void swapped();
    void on_mpv_events();
    void maybeUpdate();
private:
    void handle_mpv_event(mpv_event *event);
    static void on_update(void *ctx);

    QElapsedTimer fps;
    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *mpv_gl;

};

class DanmakuPlayer : public MpvEmWidget
{
    Q_OBJECT
public:
    DanmakuPlayer(QStringList args, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DanmakuPlayer();
    QSize sizeHint() const { return QSize(1280, 720);}

    bool isDanmakuVisible();
    void checkVideoResolution();

protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void closeDanmaku();
    void refreshStream();

private:
    QString namedPipe;
    QStringList args;
    QTimer* checkVideoResolutionTimer;
    QThread* danmakuThread;
    bool danmakuShowFlag = true;
    DanmakuLauncher *danmakuLauncher = nullptr;
    DanmakuGLWidget *danmakuGLWidget = nullptr;
};


#endif // PLAYERWINDOW_H
