#ifndef QLPHELPER_H
#define QLPHELPER_H

#include <QObject>
#include "mpvcontrol.h"
#include "ffmpegcontrol.h"
#include "streamfinder.h"
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
    FFmpegControl* ffmpeg_control = nullptr;
    StreamFinder* stream_finder = nullptr;
    DanmakuLauncher* danmaku_launcher = nullptr;
    bool reloading = false;
    bool streaming = false;
    QString stream_socket;
    QString danmaku_socket;
    QFile* ff2mpv_fifo = nullptr;
    QString room_url;
    QString record_file;
    bool is_debug = false;
    bool strict_stream = false;
};

#endif // QLPHELPER_H
