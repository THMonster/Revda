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
private:
    QStringList args;
    QString namedPipe;
    bool streamReady = false;
    QTime time;
    QTimer* mTimer;
    QProcess* dmcPyProcess;
    QProcess* streamlinkProcess;
    MpvWidget* mpvWidget;
    DanmakuRecorder* danmakuRecorder;
    int danmakuTimeNodeSeq[24] = {0};
    int danmakuTimeLengthSeq[24]= {0};
};

#endif // CLIRECORDER_H
