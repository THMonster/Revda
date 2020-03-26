#include "ffmpegcontrol.h"

FfmpegControl::FfmpegControl(QString stream_socket, QString danmaku_socket, QFile *ff2mpv_fifo, bool is_debug, bool is_hls, QObject *parent)
    : QObject(parent)
{
    ff_proc = new QProcess(this);
    this->is_debug = is_debug;
    this->is_hls = is_hls;
    this->stream_socket_path = stream_socket;
    this->danmaku_socket_path = danmaku_socket;
    this->ff2mpv_fifo = ff2mpv_fifo;
}

FfmpegControl::~FfmpegControl()
{
    ff_proc->terminate();
    ff_proc->waitForFinished(3000);
}

void FfmpegControl::start()
{
    if (is_debug) {
        ff_proc->start("ffmpeg", QStringList() << "-report" << "-y" << "-i"
                       << "unix://" + stream_socket_path << "-i" << "unix://" + danmaku_socket_path << "-map" << "0:v"
                       << "-map" << "0:a" << "-map" << "1:0" << "-c" << "copy" << "-f" << "matroska"
                       << ff2mpv_fifo->fileName());

    } else {
        ff_proc->start("ffmpeg", QStringList() << "-loglevel" << "quiet" << "-y" << "-i"
                       << "unix://" + stream_socket_path << "-i" << "unix://" + danmaku_socket_path << "-map" << "0:v"
                       << "-map" << "0:a" << "-map" << "1:0" << "-c" << "copy" << "-f" << "matroska"
                       << ff2mpv_fifo->fileName());
    }

//    ff_proc->start("sh", QStringList() << "-c" << QString("cat %2 | ffmpeg %4 -y -i %1 -i - -map 0:v -map 0:a"
//                                                          " -map 1:0 -c copy -f matroska %3")
//                   .arg(stream_fifo->fileName()).arg(danmaku_fifo->fileName()).arg(ff2mpv_fifo->fileName())
//                   .arg(is_debug ? "-report" : "-loglevel quiet"));
    ff_proc->waitForStarted();
}

void FfmpegControl::restart()
{
    ff_proc->terminate();
    qDebug() << "waiting for ffmpeg exit!";
    ff_proc->waitForFinished();
    if (is_debug) {
        ff_proc->start("ffmpeg", QStringList() << "-report" << "-y" << "-i"
                       << "unix://" + stream_socket_path << "-i" << "unix://" + danmaku_socket_path << "-map" << "0:v"
                       << "-map" << "0:a" << "-map" << "1:0" << "-c" << "copy" << "-f" << "matroska"
                       << ff2mpv_fifo->fileName());
    } else {
        ff_proc->start("ffmpeg", QStringList() << "-loglevel" << "quiet" << "-y" << "-i"
                       << "unix://" + stream_socket_path << "-i" << "unix://" + danmaku_socket_path << "-map" << "0:v"
                       << "-map" << "0:a" << "-map" << "1:0" << "-c" << "copy" << "-f" << "matroska"
                       << ff2mpv_fifo->fileName());
    }
    ff_proc->waitForStarted();
}

void FfmpegControl::setTitle(QString title)
{
    this->title = title;
}
