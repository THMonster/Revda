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
    CLIRecorder(QStringList args);
    ~CLIRecorder();
    void readDanmaku();
    void checkVideoResolution();
    int getAvailDanmakuChannel();
    void startStreamlinkProcess();
    void onStreamlinkFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void checkStreamAvailable();
public slots:
    void onStreamlinkStderrReady();
private:
    QStringList args;
    QString namedPipe = "";
    bool streamAvailable = false;
    bool streamReady = false;
    int pausedTime = 0;
    QTime time;
    QTimer* checkStreamReadyTimer;
    QProcess* dmcPyProcess;
    QProcess* streamlinkProcess;
    QProcess* checkProcess;
    MpvWidget* mpvWidget;
    DanmakuRecorder* danmakuRecorder;
    int danmakuTimeNodeSeq[24] = {0};
    int danmakuTimeLengthSeq[24]= {0};
    int videoPart = 1;
};

#endif // CLIRECORDER_H
