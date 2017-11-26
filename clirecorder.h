#ifndef CLIRECORDER_H
#define CLIRECORDER_H
#include "danmakurecorder.h"
#include "mpvwidget.h"
#include <QtGui>
class QObject;
class CLIRecorder : public QObject
{
    Q_OBJECT
public:
    CLIRecorder();
    ~CLIRecorder();
    void readDanmaku();
    void checkVideoResolution();
    int getAvailDanmakuChannel();
private:
    bool streamReady = false;
    QTime time;
    QTimer* mTimer;
    QProcess* mProcess;
    MpvWidget* mpvWidget;
    DanmakuRecorder* danmakuRecorder;
    int danmakuTimeNodeSeq[24] = {0};
    int danmakuTimeLengthSeq[24]= {0};
};

#endif // CLIRECORDER_H
