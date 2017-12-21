#ifndef DANMAKULAUNCHER_H
#define DANMAKULAUNCHER_H
#include <QtGui>
#include <QLabel>
#include "mpvwidget.h"
#include "danmakuglwidget.h"

struct Danmaku_t
{
    QString speaker;
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
    DanmakuLauncher(QStringList args, QWidget *parent = 0, int resWidth = 0, int resHeight = 0);
    ~DanmakuLauncher();
    void initDmcPy();
    void initDRecorder();
    void launchDanmaku();
    int getAvailDanmakuChannel(double currentSpeed);
    void paintDanmaku(QPainter *painter);
    void paintDanmakuCLI();
    void initDL();
    void clearDanmakuQueue();
    void setDanmakuShowFlag(bool flag);
    void setStreamReadyFlag(bool flag);
    void updateResolution();
    void setPlayingState(int state);
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
    DanmakuRecorder* danmakuRecorder;
    int danmakuTimeNodeSeq[24] = {0};
    double danmakuWidthSeq[24]= {0};
    double danmakuSpeedSeq[24] = {0};
    QTime time;
    bool danmakuShowFlag = true;
    bool streamReady = false;
    QPen borderPen;
    QPen textPen;
    QFont font;
    int resWidth = 0;
    int resHeight = 0;
};

#endif // DANMAKULAUNCHER_H
