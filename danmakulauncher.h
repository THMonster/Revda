#ifndef DANMAKULAUNCHER_H
#define DANMAKULAUNCHER_H
#include <QtGui>
#include <QLabel>
#include "mpvwidget.h"
#include "danmakuglwidget.h"

struct Danmaku_t
{
    QString text;
    double posX;
    double posY;
    double length;
    double step;
};

class DanmakuGLWidget;
class DanmakuPlayer;
class DanmakuLauncher : public QObject
{
    Q_OBJECT
public:
    DanmakuLauncher(QStringList args, QWidget *parent = 0);
    ~DanmakuLauncher();
    void initDmcPy();
    void launchDanmaku();
    int getAvailDanmakuChannel(double currentSpeed);
    void paintDanmaku(QPainter *painter);
    void initDL();
    void clearDanmakuQueue();
    void setDanmakuShowFlag(bool flag);
signals:
    void sendDanmaku(QString danmakuText, int durationMs, int y);

private:
    QMutex mutex;
    QQueue<Danmaku_t> danmakuQueue;
    QTimer* launchDanmakuTimer;
    QTimer* paintTimer;
    QWidget* dglw;
    QStringList args;
    QProcess* dmcPyProcess;
    int danmakuTimeNodeSeq[24] = {0};
    double danmakuWidthSeq[24]= {0};
    double danmakuSpeedSeq[24] = {0};
    QTime time;
    bool danmakuShowFlag = true;
//    QPainterPath textPath;
//    QBrush textBrush;
    QPen borderPen;
    QPen textPen;
    QFont font;
};

#endif // DANMAKULAUNCHER_H
