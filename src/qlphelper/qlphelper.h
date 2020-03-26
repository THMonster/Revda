#ifndef QLPHELPER_H
#define QLPHELPER_H

#include <QObject>
#include "mpvcontrol.h"
#include "ffmpegcontrol.h"
#include "streamer.h"
#include "danmakulauncher.h"

class QLPHelper : public QObject
{
    Q_OBJECT
public:
    explicit QLPHelper(QStringList args, QObject *parent = nullptr);
    ~QLPHelper();

public slots:
    void start();
    void restart();
    void restarted();

signals:

private:
    MpvControl* mpv_control = nullptr;
    FfmpegControl* ffmpeg_control = nullptr;
    Streamer* streamer = nullptr;
    DanmakuLauncher* danmaku_launcher = nullptr;
    bool reloading = false;
    bool streaming = false;
    QString stream_socket;
    QString danmaku_socket;
    QFile* ff2mpv_fifo = nullptr;
    QString room_url;
    QString record_file;
    bool is_debug = false;
};

#endif // QLPHELPER_H
