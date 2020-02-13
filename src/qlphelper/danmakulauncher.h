#ifndef DANMAKULAUNCHER_H
#define DANMAKULAUNCHER_H
#include <QtCore>

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
    DanmakuLauncher(QStringList args);
    ~DanmakuLauncher();
    void initDmcPy();
    void initPlayer();
    void setScale();
    QString getPlayerCMD(QString url);
    int getDankamuDisplayLength(QString dm, int fontsize);
    void loadDanmaku();
    void launchDanmaku();
    void launchVoidDanmaku();
    int getAvailDanmakuChannel(int len);
    void printPlayerProcess();
    void getLiveStatus();
    void launchLiveChecker();
    QString getStreamUrl(QString url);

private:
    bool on_buffering = true;
    int channel_num = 20;
    int font_size = 40;
    double scale = 1.0;
    qint64 speed = 8000; // duration of each danmaku in ms
    QFile *fifo_file = nullptr;
    QDataStream *out = nullptr;
    QTimer *launch_timer = nullptr;
    QString url;
    QString record_file;
    bool no_window = false;
    bool is_debug = false;
    QString stream_url;
    QString title;
    QString fifo;
    QElapsedTimer timer;
    qint64 pts = 0;
    quint64 read_order = 0;
    quint8 check_counter = 0;
    QQueue<QString> danmaku_queue;
    QList<QPair<QString, QString>> droped_danmaku_list;
    DanmakuChannel danmaku_channel[30];
    QProcess* dmcPyProcess;
    QProcess* player;
    QProcess* live_checker;
    FudujiKiller *fk = nullptr;
};

#endif // DANMAKULAUNCHER_H
