#ifndef DANMAKULAUNCHER_H
#define DANMAKULAUNCHER_H
#include <QtGui>
#include <QLabel>
#include "mpvwidget.h"

class DanmakuPlayer;
class DanmakuLauncher : public QObject
{
    Q_OBJECT
public:
    DanmakuLauncher(QStringList args, DanmakuPlayer* parent = 0);
    ~DanmakuLauncher();
    void initDmcPy();
    void launchDanmaku();
    int getAvailDanmakuChannel();

signals:
    void sendDanmaku(QString danmakuText, int durationMs, int y);

private:
    QTimer* launchDanmakuTimer;
    DanmakuPlayer* dmp;
    QStringList args;
    QProcess* dmcPyProcess;
    int danmakuTimeNodeSeq[24] = {0};
    int danmakuTimeLengthSeq[24]= {0};
    QTime time;
};

#endif // DANMAKULAUNCHER_H
