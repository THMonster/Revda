#ifndef DANMAKULAUNCHER_H
#define DANMAKULAUNCHER_H
#include <QtCore>
#include <QLocalServer>
#include <QLocalSocket>

#include "fudujikiller.h"

struct DanmakuChannel
{
//    qint64 duration;
    int length;
    qint64 begin_pts;
};

class DanmakuLauncher : public QObject
{
    Q_OBJECT
public:
    DanmakuLauncher(QString room_url, QString danmaku_socket);
    ~DanmakuLauncher();
    void startDmcPy();
    int getDankamuDisplayLength(QString dm, int fontsize);
    void loadDanmaku();
    void launchDanmaku();
    void launchVoidDanmaku();
    int getAvailDanmakuChannel(int len);

public slots:
    void start();
    void restart();
    void stop();
    void onStreamStart();
    void setSocket();
    void setScale(int w, int h);

private:
    int channel_num = 14;
    int font_size = 40;
    double scale = 1.0;
    qint64 speed = 8000; // duration of each danmaku in ms
    QString danmaku_socket_path;
    QTimer *launch_timer = nullptr;
    QString room_url;
    QElapsedTimer timer;
    qint64 pts = 0;
    quint64 read_order = 0;
    QQueue<QString> danmaku_queue;
    QList<QStringList> droped_danmaku_list;
    DanmakuChannel danmaku_channel[30];
    QProcess* dmcPyProcess;
    FudujiKiller *fk = nullptr;
    bool on_streaming = false;
    QLocalServer* socket_server = nullptr;
    QLocalSocket* socket = nullptr;
};

#endif // DANMAKULAUNCHER_H
