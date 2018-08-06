#ifndef DANMAKUGLWIDGET_H
#define DANMAKUGLWIDGET_H

#include <QtGui>
#include <QtCore>
#include <QOpenGLWidget>
#include <QThread>
#include "danmakulauncher.h"

class DanmakuLauncher;
class DanmakuGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit DanmakuGLWidget(QStringList args, QWidget *parent = nullptr);
    ~DanmakuGLWidget();

protected:
//    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
//    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;


signals:

public slots:
    void setStreamReadyFlag(bool flag);

private:
    DanmakuLauncher *danmakuLauncher = nullptr;
    QThread *danmakuThread;
    QTimer *updateTimer = nullptr;

};

#endif // DANMAKUGLWIDGET_H
