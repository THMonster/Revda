#ifndef DANMAKULAUNCHER_H
#define DANMAKULAUNCHER_H
#include <QtCore>

struct DanmakuChannel
{
    qint64 duration;
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
    QString getPlayerCMD(QString url);
    int getDankamuDisplayLength(QString dm, int fontsize);
    void loadDanmaku();
    void launchDanmaku();
    void launchVoidDanmaku();
    int getAvailDanmakuChannel(double speed);
    void printPlayerProcess();
    void getLiveStatus();
    void launchLiveChecker();
    QString getStreamUrl(QString url);

private:
    bool on_buffering = true;
    int channel_num = 20;
    int font_size = 40;
    double speed_factor = 1.0;
    QFile *fifo_file = nullptr;
    QDataStream *out = nullptr;
    QTimer *launch_timer = nullptr;
    QString url;
    QString record_file;
    QString stream_url;
    QString title;
    QString fifo;
    QElapsedTimer timer;
    qint64 pts = 0;
    quint64 read_order = 0;
    quint8 check_counter = 0;
    QQueue<QString> danmaku_queue;
    DanmakuChannel danmaku_channel[30];
    QProcess* dmcPyProcess;
    QProcess* player;
    QProcess* live_checker;
};

#endif // DANMAKULAUNCHER_H
