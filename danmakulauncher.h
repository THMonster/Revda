#ifndef DANMAKULAUNCHER_H
#define DANMAKULAUNCHER_H
#include <QtGui>
#include <QLabel>
#include "mpvwidget.h"
#include "danmakuglwidget.h"

struct Danmaku_t
{
    QString text;
    int posX;
    int posY;
    float step;
};
class DanmakuGLWidget;
class DanmakuPlayer;
class DanmakuLauncher : public QObject
{
    Q_OBJECT
public:
    DanmakuLauncher(QStringList args, DanmakuGLWidget* parent = 0);
    ~DanmakuLauncher();
    void initDmcPy();
    void launchDanmaku();
    int getAvailDanmakuChannel();
    void paintDanmaku(QPainter *painter, QPaintEvent *event);
    void initDL();
signals:
    void sendDanmaku(QString danmakuText, int durationMs, int y);

private:
    QMutex mutex;
    QQueue<Danmaku_t> danmakuQueue;
    QTimer* launchDanmakuTimer;
    QTimer* paintTimer;
    DanmakuGLWidget* dglw;
    QStringList args;
    QProcess* dmcPyProcess;
    int danmakuTimeNodeSeq[24] = {0};
    int danmakuTimeLengthSeq[24]= {0};
    QTime time;

    QPen textPen;
    QFont font;
};

#endif // DANMAKULAUNCHER_H
