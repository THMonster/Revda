#ifndef CLIRECORDER_H
#define CLIRECORDER_H
#include "danmakurecorder.h"
#include "danmakuplayer.h"
#include <QtGui>
class QObject;
class CLIClient : public QObject
{
    Q_OBJECT
public:
    CLIClient(QStringList args);
    ~CLIClient();
    void checkStreamReady();
    void startStreamlinkProcess();
    void onStreamlinkFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void checkStreamAvailable();
    void startDanmakuLauncher();
public slots:
    void onStreamlinkStderrReady();
private:
    QThread* danmakuLauncherThread;
    QStringList args;
    QString namedPipe = "";
    bool streamAvailable = false;
    bool streamReady = false;
    bool boardcastStarted = false;
    QTime time;
    QTimer* checkStreamReadyTimer;
    QTimer* paintTimer;
    QProcess* streamlinkProcess;
    QProcess* checkProcess;
    DanmakuLauncher* danmakuLauncher;
    int videoPart = 1;
};

#endif // CLIRECORDER_H
