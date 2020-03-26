#ifndef FFMPEGCONTROL_H
#define FFMPEGCONTROL_H

#include <QtCore>

class FfmpegControl : public QObject
{
    Q_OBJECT
public:
    explicit FfmpegControl(QString stream_socket, QString danmaku_socket, QFile* ff2mpv_fifo, bool is_debug = false, bool is_hls = false, QObject *parent = nullptr);
    ~FfmpegControl();

public slots:
    void start();
    void restart();
    void setTitle(QString title);

signals:

private:
    QProcess* ff_proc = nullptr;
    bool is_debug = false;
    QString stream_socket_path;
    QString danmaku_socket_path;
    QFile* ff2mpv_fifo = nullptr;
    QString title;
    bool is_hls = false;
};

#endif // FFMPEGCONTROL_H
