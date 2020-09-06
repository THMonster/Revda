#include "ffmpegcontrol.h"

FFmpegControl::FFmpegControl(QString stream_socket, QString danmaku_socket, QFile *ff2mpv_fifo, QString record_file, bool is_debug, bool strict_stream, QObject *parent)
    : QObject(parent)
{
    ff_proc = new QProcess(this);
    this->is_debug = is_debug;
    this->strict_stream = strict_stream;
    this->stream_socket_path = stream_socket;
    this->danmaku_socket_path = danmaku_socket;
    this->ff2mpv_fifo = ff2mpv_fifo;
    this->record_file = record_file;
}

FFmpegControl::~FFmpegControl()
{
    ff_proc->waitForFinished(3000);
    ff_proc->terminate();
}

void FFmpegControl::start()
{
//    ff_proc->start("ffmpeg", getFFmpegCmdline());
//    ff_proc->waitForStarted();
}

void FFmpegControl::restart()
{
    qDebug() << "waiting for ffmpeg exit!";
    ff_proc->waitForFinished(3000);
    ff_proc->terminate();
    ff_proc->waitForFinished();
//    ff_proc->start("ffmpeg", getFFmpegCmdline());
//    ff_proc->waitForStarted();
    ready_flag = 0;
}

void FFmpegControl::stop()
{
    qDebug() << "waiting for ffmpeg exit!";
    ff_proc->waitForFinished(3000);
    ff_proc->terminate();
    ff_proc->waitForFinished();
    ready_flag = 0;
}

void FFmpegControl::setTitle(QString title)
{
    this->title = title;
}

void FFmpegControl::onStreamReady()
{
    if (ready_flag >= 1) {
        return;
    }
    ready_flag = ready_flag | 1;
    if (ready_flag >= 1) {
        ff_proc->start("ffmpeg", getFFmpegCmdline());
        ff_proc->waitForStarted();
    }
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
        << "unix://" + danmaku_socket_path << "-map" << "0:v:0"
        << "-map" << "0:a:0" << "-map" << "1:s:0" << "-c" << "copy"
        << "-f" << "matroska";
    if (record_file.isEmpty() || true) {
        ret.append(ff2mpv_fifo->fileName());
    } else {
        ret.append(genRecordFileName());
    }
    return ret;
}

QString FFmpegControl::genRecordFileName()
{
    return record_file + "." + QString::number(record_cnt++);
}
