#ifndef DANMAKULAUNCHER_H
#define DANMAKULAUNCHER_H
#include <QLocalServer>
#include <QLocalSocket>
#include <QtCore>

#include "../Binding.h"
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

    enum State
    {
        NotRunning,
        WaitingForStream,
        WaitingForSocket,
        Running
    } state = NotRunning;

  public:
    DanmakuLauncher(QString room_url,
                    QString danmaku_socket,
                    double fs = -1,
                    double fa = -1,
                    int speed = 8000,
                    bool quiet = false,
                    QObject* parent = nullptr);
    ~DanmakuLauncher();
    void fetchDanmaku();
    void stopDanmakuClient();
    int getDankamuDisplayLength(QString dm, int fontsize);
    void loadDanmaku();
    void launchDanmaku();
    void launchVoidDanmaku(int cnt = 1);
    int getAvailDanmakuChannel(int len);

  public slots:
    void start();
    void restart();
    void stop();
    void onStreamStart();
    void setSocket();
    void setScale(int w, int h);
    void setFont(double fs, double fa);
    void setFontScaleDelta(double delta);
    void setToggleNick();
    void setSpeed(int ms);

  private:
    int channel_num = 14;
    int font_size = 40;
    double scale = 1.0;
    int speed; // duration of each danmaku in ms
    QString danmaku_socket_path;
    QTimer* launch_timer = nullptr;
    QString room_url;
    QElapsedTimer timer;
    qint64 pts = 0;
    quint64 read_order = 0;
    QQueue<QString> danmaku_queue;
    QList<QStringList> dropped_danmaku_list;
    DanmakuChannel danmaku_channel[30];
    QProcess* dmcPyProcess;
    FudujiKiller* fk = nullptr;
    bool on_streaming = false;
    QLocalServer* socket_server = nullptr;
    QLocalSocket* socket = nullptr;
    QString font_alpha = QStringLiteral("00");
    bool show_nick = false;
    bool quiet = false;
    QLivePlayerLib* qlp_lib = nullptr;
};

#endif // DANMAKULAUNCHER_H
