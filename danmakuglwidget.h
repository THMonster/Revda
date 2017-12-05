#ifndef DANMAKUGLWIDGET_H
#define DANMAKUGLWIDGET_H
#include "danmakulauncher.h"
#include <QtGui>
#include <QOpenGLWidget>
class DanmakuLauncher;
class DanmakuGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    DanmakuGLWidget(QStringList args, QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~DanmakuGLWidget();
public slots:
    void animate();


protected:
    void paintEvent(QPaintEvent *event) override;

private:
    DanmakuLauncher* danmakuLauncher;
    QThread* danmakuThread;
};

#endif // DANMAKUGLWIDGET_H
