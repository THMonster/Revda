#include "ffmpegcontrol.h"

FFmpegControl::FFmpegControl(QString stream_socket, QString danmaku_socket, QFile *ff2mpv_fifo, bool is_debug, bool strict_stream, QObject *parent)
    : QObject(parent)
{
    ff_proc = new QProcess(this);
    this->is_debug = is_debug;
    this->strict_stream = strict_stream;
    this->stream_socket_path = stream_socket;
    this->danmaku_socket_path = danmaku_socket;
    this->ff2mpv_fifo = ff2mpv_fifo;
}

FFmpegControl::~FFmpegControl()
{
    ff_proc->terminate();
    ff_proc->waitForFinished(3000);
}

void FFmpegControl::start()
{
    ff_proc->start("ffmpeg", getFFmpegCmdline());
    ff_proc->waitForStarted();
}

void FFmpegControl::restart()
{
    ff_proc->terminate();
    qDebug() << "waiting for ffmpeg exit!";
    ff_proc->waitForFinished();
    ff_proc->start("ffmpeg", getFFmpegCmdline());
    ff_proc->waitForStarted();
}

void FFmpegControl::setTitle(QString title)
{
    this->title = title;
}

QStringList FFmpegControl::getFFmpegCmdline()
{
    QStringList ret;
    if (is_debug) {
        ret.append("-report");
    } else {
        ret.append("-loglevel");
        ret.append("quiet");
    }
    if (strict_stream) {
        ret.append("-xerror");
    }
    ret << "-y" << "-i" << "unix://" + stream_socket_path << "-i"
        << "unix://" + danmaku_socket_path << "-map" << "0:v"
        << "-map" << "0:a" << "-map" << "1:0" << "-c" << "copy"
        << "-f" << "matroska" << ff2mpv_fifo->fileName();
    return ret;
}
