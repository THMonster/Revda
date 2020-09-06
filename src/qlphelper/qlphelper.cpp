#include "qlphelper.h"

QLPHelper::QLPHelper(QStringList args, QObject *parent) : QObject(parent)
{
    room_url = args.at(0);
    record_file = args.at(1).compare("null") == 0 ? "" : args.at(1);
    if (args.at(2) == "true") {
        strict_stream = true;
    }
    if (args.at(3) == "true") {
        is_debug = true;
    }

    stream_socket = QString("/tmp/qlp-%1").arg(QUuid::createUuid().toString());
    danmaku_socket = QString("/tmp/qlp-%1").arg(QUuid::createUuid().toString());
    auto f = QString("/tmp/qlp-%1").arg(QUuid::createUuid().toString());
    QProcess::execute("mkfifo", QStringList() << f);
    ff2mpv_fifo = new QFile(f, this);


    ffmpeg_control = new FFmpegControl(stream_socket, danmaku_socket, ff2mpv_fifo, record_file, is_debug, strict_stream);
    mpv_control = new MpvControl(ff2mpv_fifo, record_file);
    stream_finder = new StreamFinder(room_url, stream_socket, this);
    danmaku_launcher = new DanmakuLauncher(room_url, danmaku_socket);

    connect(stream_finder, &StreamFinder::streamError, this, &QLPHelper::restart);
    connect(stream_finder, &StreamFinder::streamStart, danmaku_launcher, &DanmakuLauncher::onStreamStart);
    connect(mpv_control, &MpvControl::requestReload, this, &QLPHelper::restart);
    connect(mpv_control, &MpvControl::resFetched, danmaku_launcher, &DanmakuLauncher::setScale);
    connect(stream_finder, &StreamFinder::titleMatched, mpv_control, &MpvControl::setTitle);
    connect(stream_finder, &StreamFinder::ready, ffmpeg_control, &FFmpegControl::onStreamReady);
}

QLPHelper::~QLPHelper()
{
    qCritical() << "Bye!";
    danmaku_launcher->stop();
    stream_finder->stop();
    ffmpeg_control->stop();
    delete stream_finder;
    delete danmaku_launcher;
    delete ffmpeg_control;
    delete mpv_control;

    ff2mpv_fifo->close();
    ff2mpv_fifo->remove();

}

void QLPHelper::start()
{
    stream_finder->start();
    qDebug() << "streamer started";
    danmaku_launcher->start();
    qDebug() << "danmaku launcher started";
    ffmpeg_control->start();
    qDebug() << "ffmpeg started";
    mpv_control->start();
    qDebug() << "mpv started";
}

void QLPHelper::restart()
{
    // Be careful of the order of restart, due to ffmpeg's IO policy.
    // If the last input of ffmpeg is stucked, the whole ffmpeg will be blocked.
    danmaku_launcher->restart();
    qDebug() << "danmaku launcher started";
    stream_finder->restart();
    qDebug() << "streamer started";

    ffmpeg_control->restart();
    qDebug() << "ffmpeg started";
    mpv_control->restart();
    qDebug() << "mpv started";
}

void QLPHelper::restarted()
{
    reloading = false;
    streaming = true;
}
