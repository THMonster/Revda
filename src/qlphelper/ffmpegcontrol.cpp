#include "ffmpegcontrol.h"

FFmpegControl::FFmpegControl(QString stream_socket,
                             QString danmaku_socket,
                             QFile* ff2mpv_fifo,
                             QString record_file,
                             bool is_debug,
                             bool strict_stream,
                             QObject* parent)
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

void
FFmpegControl::start()
{
    state = Idle;
}

void
FFmpegControl::restart()
{
    qDebug() << "waiting for ffmpeg exit!";
    ff_proc->waitForFinished(3000);
    ff_proc->terminate();
    ff_proc->waitForFinished();
    state = Idle;
}

void
FFmpegControl::stop()
{
    qDebug() << "waiting for ffmpeg exit!";
    ff_proc->waitForFinished(3000);
    ff_proc->terminate();
    ff_proc->waitForFinished();
    state = Idle;
}

void
FFmpegControl::setTitle(QString title)
{
    this->title = title;
}

void
FFmpegControl::onStreamReady(QString title, int flag)
{
    this->title = title;
    if ((flag & 0x01) == 1) {
        is_hls = true;
    }
    if (state == Idle) {
        qDebug() << "start ffmpeg proc";
        ff_proc->start("ffmpeg", getFFmpegCmdline());
        ff_proc->waitForStarted();
        state = Running;
    }
}

QStringList
FFmpegControl::getFFmpegCmdline()
{
    QStringList ret;
    ret.append("-y");
    if (is_debug) {
        ret.append("-report");
    } else {
        ret << "-loglevel"
            << "quiet";
    }
    if (strict_stream || is_hls) {
        ret.append("-xerror");
    }
    ret << "-i"
        << "unix://" + stream_socket_path;
    ret << "-i"
        << "unix://" + danmaku_socket_path;
    ret << "-map"
        << "0:v:0"
        << "-map"
        << "0:a:0"
        << "-map"
        << "1:s:0";
    ret << "-c"
        << "copy";
    if (is_hls == true) {
        // Error parsing AAC extradata, unable to determine samplerate
        ret << "-c:a"
            << "pcm_s16le";
    }
    ret << "-metadata"
        << "title=" + title;
    ret << "-f"
        << "matroska";
    if (record_file.isEmpty() || true) {
        ret.append(ff2mpv_fifo->fileName());
    } else {
        ret.append(genRecordFileName());
    }
    return ret;
}

inline QString
FFmpegControl::genRecordFileName()
{
    return record_file + "." + QString::number(record_cnt++);
}
