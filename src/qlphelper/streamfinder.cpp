#include "streamfinder.h"
#include "../Binding.h"

StreamFinder::StreamFinder(QString room_url, QString stream_socket, QObject* parent)
  : QObject(parent)
{
    this->room_url = room_url;
    this->stream_socket = stream_socket;
}

StreamFinder::~StreamFinder()
{
    if (streamer != nullptr) {
        streamer->close();
        streamer->deleteLater();
        streamer = nullptr;
    }
}

void
StreamFinder::start()
{
    startRequest();
}

void
StreamFinder::restart()
{
    real_url.clear();
    if (streamer != nullptr) {
        streamer->close();
        streamer->deleteLater();
        streamer = nullptr;
    }
    startRequest();
}

void
StreamFinder::stop()
{
    real_url.clear();
    if (streamer != nullptr) {
        streamer->close();
        streamer->deleteLater();
        streamer = nullptr;
    }
}

void
StreamFinder::setQuality(int q)
{
    quality = q;
    emit streamError();
}

void
StreamFinder::startRequest()
{
    qInfo() << "Finding stream...";
    QLivePlayerLib qlib;
    auto res = qlib.get_url(room_url, QString());
    //    qInfo() << res;
    if (res.startsWith("qlp_nostream")) {
        qInfo() << "Stream url not found!";
        if (offline_counter > 30) {
            QCoreApplication::exit(0);
        } else {
            ++offline_counter;
            QTimer::singleShot(2000, this, [this]() {
                startRequest();
            });
        }
    } else {
        offline_counter = 0;
        this->title.clear();
        this->real_url.clear();
        int i = 0;
        for (const auto& u : res.split("\n", Qt::SkipEmptyParts)) {
            if (i == 0) {
                this->title = u;
            } else {
                this->real_url.append(u);
                this->real_url.append('\n');
            }
            ++i;
        }
        this->real_url.chop(1);
        emit titleMatched(title);
        startStreamer();
    }
}

void
StreamFinder::startStreamer()
{
    qInfo().noquote() << "Playing: " << title;
    if (room_url.contains("youtube.com/")) {
        streamer = new StreamerDash(real_url, stream_socket, this);
        connect(streamer, &Streamer::streamError, this, [this]() {
            emit this->streamError();
        });
        connect(streamer, &Streamer::streamStart, this, [this]() {
            emit this->streamStart();
        });
        streamer->start();
        emit ready(title, 0x02);
    } else if (real_url.contains(".m3u8")) {
        streamer = new StreamerHls(real_url, stream_socket, this);
        connect(streamer, &Streamer::streamError, this, [this]() {
            emit this->streamError();
        });
        connect(streamer, &Streamer::streamStart, this, [this]() {
            emit this->streamStart();
        });
        streamer->start();
        emit this->ready(title, 0x01);
    } else {
        streamer = new StreamerFlv(real_url, room_url, stream_socket, this);
        connect(streamer, &Streamer::streamError, this, [this]() {
            emit this->streamError();
        });
        connect(streamer, &Streamer::streamStart, this, [this]() {
            emit this->streamStart();
        });
        streamer->start();
        emit ready(title, 0);
    }
}
