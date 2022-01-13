#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>

#include "streamer.h"

#define qsl(s) QStringLiteral(s)

StreamerFlv::StreamerFlv(QString real_url, QString room_url, QString socket_path, QObject* parent)
  : real_url(real_url)
  , room_url(room_url)
  , stream_socket_path(socket_path)
{
    qlp_lib = QSharedPointer<QLivePlayerLib>(new QLivePlayerLib());
    connect(this, &StreamerFlv::qlp_streamer_finished, this, &StreamerFlv::setError, Qt::QueuedConnection);
    connect(
      this, &StreamerFlv::qlp_streamer_stream_started, this,
      [this]() {
          if (state == Idle) {
              state = Running;
              emit streamStart();
              qDebug() << "stream started!";
          }
      },
      Qt::QueuedConnection);
}

StreamerFlv::~StreamerFlv()
{
    qDebug() << "StreamerFlv deleted";
    close();
}

void
StreamerFlv::start()
{
    if (state == Idle) {
        auto self = this->sharedFromThis();
        auto fu = QtConcurrent::run(this, &StreamerFlv::qlp_run_streamer);
        auto fu1 = QtConcurrent::run(this, &StreamerFlv::qlp_check_streamer_loading);
    }
}

void
StreamerFlv::close()
{
    state = Closing;
    qlp_lib->stop_danmaku_client();
}

void
StreamerFlv::setError()
{
    if (state != Closing) {
        qDebug() << "set error";
        emit streamError();
        state = Closing;
    }
}

void
StreamerFlv::qlp_run_streamer()
{
    auto self = this->sharedFromThis();
    qlp_lib->run_streamer("flv", this->real_url, this->stream_socket_path + "\n" + this->room_url);
    emit qlp_streamer_finished();
}

void
StreamerFlv::qlp_check_streamer_loading()
{
    auto self = this->sharedFromThis();
    qlp_lib->check_streamer_loading();
    emit qlp_streamer_stream_started();
}

StreamerHls::StreamerHls(QString real_url, QString socket_path, QObject* parent)
  : real_url(real_url)
  , stream_socket_path(socket_path)
{
    qlp_lib = QSharedPointer<QLivePlayerLib>(new QLivePlayerLib());
    connect(this, &StreamerHls::qlp_streamer_finished, this, &StreamerHls::setError, Qt::QueuedConnection);
    connect(
      this, &StreamerHls::qlp_streamer_stream_started, this,
      [this]() {
          if (state == Idle) {
              state = Running;
              emit streamStart();
              qDebug() << "stream started!";
          }
      },
      Qt::QueuedConnection);
}

StreamerHls::~StreamerHls()
{
    qDebug() << "StreamerHls deleted";
    close();
}

void
StreamerHls::start()
{
    if (state == Idle) {
        auto self = this->sharedFromThis();
        auto fu = QtConcurrent::run(this, &StreamerHls::qlp_run_streamer);
        auto fu1 = QtConcurrent::run(this, &StreamerHls::qlp_check_streamer_loading);
    }
}

void
StreamerHls::close()
{
    state = Closing;
    qlp_lib->stop_danmaku_client();
}

void
StreamerHls::setError()
{
    if (state != Closing) {
        qDebug() << "set error";
        emit streamError();
        state = Closing;
    }
}

void
StreamerHls::qlp_run_streamer()
{
    auto self = this->sharedFromThis();
    qlp_lib->run_streamer("hls", this->real_url, this->stream_socket_path);
    emit qlp_streamer_finished();
}

void
StreamerHls::qlp_check_streamer_loading()
{
    auto self = this->sharedFromThis();
    qlp_lib->check_streamer_loading();
    emit qlp_streamer_stream_started();
}

StreamerDash::StreamerDash(QString real_url, QString socket_path, QObject* parent)
  : real_url(real_url)
  , stream_socket_path(socket_path)
{
    qlp_lib = QSharedPointer<QLivePlayerLib>(new QLivePlayerLib());
    connect(this, &StreamerDash::qlp_streamer_finished, this, &StreamerDash::setError, Qt::QueuedConnection);
    connect(
      this, &StreamerDash::qlp_streamer_stream_started, this,
      [this]() {
          if (state == Idle) {
              state = Running;
              emit streamStart();
              qDebug() << "stream started!";
          }
      },
      Qt::QueuedConnection);
}

StreamerDash::~StreamerDash()
{
    qDebug() << "StreamerDash deleted";
    close();
}

void
StreamerDash::start()
{
    if (state == Idle) {
        auto self = this->sharedFromThis();
        auto fu = QtConcurrent::run(this, &StreamerDash::qlp_run_streamer);
        auto fu1 = QtConcurrent::run(this, &StreamerDash::qlp_check_streamer_loading);
    }
}

void
StreamerDash::close()
{
    state = Closing;
    qlp_lib->stop_danmaku_client();
}

void
StreamerDash::setError()
{
    if (state != Closing) {
        qDebug() << "set error";
        emit streamError();
        state = Closing;
    }
}

void
StreamerDash::qlp_run_streamer()
{
    auto self = this->sharedFromThis();
    qlp_lib->run_streamer("youtube", this->real_url, this->stream_socket_path);
    emit qlp_streamer_finished();
}

void
StreamerDash::qlp_check_streamer_loading()
{
    auto self = this->sharedFromThis();
    qlp_lib->check_streamer_loading();
    emit qlp_streamer_stream_started();
}
