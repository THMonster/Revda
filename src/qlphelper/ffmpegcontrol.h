#ifndef FFMPEGCONTROL_H
#define FFMPEGCONTROL_H

#include <QtCore>

class FFmpegControl : public QObject
{
    Q_OBJECT
  public:
    explicit FFmpegControl(QString stream_socket,
                           QString danmaku_socket,
                           QFile* ff2mpv_fifo,
                           QString record_file,
                           bool is_debug = false,
                           bool strict_stream = false,
                           QObject* parent = nullptr);
    ~FFmpegControl();

    enum State
    {
        Idle,
        Ready,
        Running
    } state = Idle;

  public slots:
    void start();
    void restart();
    void stop();
    void setTitle(QString title);
    void onStreamReady(QString title, int flag = 0);

  signals:

  private:
    QProcess* ff_proc = nullptr;
    bool is_debug = false;
    QString stream_socket_path;
    QString danmaku_socket_path;
    QString record_file;
    int record_cnt = 0;
    QFile* ff2mpv_fifo = nullptr;
    QString title;
    bool strict_stream = false;
    int ready_flag = 0;
    bool is_hls = false;
    bool is_dash = false;

    QStringList getFFmpegCmdline();
    QString genRecordFileName();
};

#endif // FFMPEGCONTROL_H
