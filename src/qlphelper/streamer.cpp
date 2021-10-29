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
    proc = new QProcess(this);
    connect(proc, &QProcess::readyReadStandardOutput, this, &StreamerFlv::onProcStdout);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &StreamerFlv::onProcFinished);
}

StreamerFlv::~StreamerFlv()
{
    qDebug() << "StreamerFlv deleted";
    close();
}

void
StreamerFlv::start()
{
    state = Idle;
    socket_server = new QLocalServer(this);
    connect(socket_server, &QLocalServer::newConnection, this, &StreamerFlv::setSocket);
    socket_server->listen(stream_socket_path);
}

void
StreamerFlv::close()
{
    state = Closing;
    proc->terminate();
    proc->waitForFinished();
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    if (socket_server != nullptr) {
        socket_server->close();
    }
}

void
StreamerFlv::setSocket()
{
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    socket = socket_server->nextPendingConnection();
    connect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), this,
            [this](QLocalSocket::LocalSocketError socketError) {
                qDebug() << "stream socket error: " << socketError;
                if (state != Closing) {
                    emit streamError();
                }
            });
    requestStream();
}

void
StreamerFlv::onProcStdout()
{
    if (state == Idle) {
        state = Running;
        emit streamStart();
        qDebug() << "stream started!";
    }
    socket->write(proc->readAllStandardOutput());
}

void
StreamerFlv::onProcFinished(int code, QProcess::ExitStatus es)
{
    Q_UNUSED(code);
    Q_UNUSED(es);

    qDebug() << "curl exited";
    if (state != Closing) {
        emit streamError();
    }
}

void
StreamerFlv::requestStream()
{
    qDebug() << real_url;
    proc->terminate();
    proc->waitForFinished();
    quint32 v = QRandomGenerator::global()->bounded(21) + 68;
    proc->start(
      "curl", QStringList() << "-H" << qsl("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:%1.0) Gecko/20100101 Firefox/%2.0").arg(v).arg(v)
                            << "-H" << qsl("Referer: %1").arg(room_url) << "--speed-limit"
                            << "1000"
                            << "--speed-time"
                            << "5"
                            << "-s"
                            << "-L" << real_url);
}

StreamerHls::StreamerHls(QString real_url, QString socket_path, QObject* parent)
  : real_url(real_url)
  , stream_socket_path(socket_path)
  , ua("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36")
{
    nam = new QNetworkAccessManager(this);
    nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

StreamerHls::~StreamerHls()
{
    qDebug() << "StreamerHls deleted";
    close();
}

void
StreamerHls::start()
{
    state = Idle;
    socket_server = new QLocalServer(this);
    connect(socket_server, &QLocalServer::newConnection, this, &StreamerHls::setSocket);
    socket_server->listen(stream_socket_path);
}

void
StreamerHls::close()
{
    state = Closing;
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    if (socket_server != nullptr) {
        socket_server->close();
    }
}

void
StreamerHls::setSocket()
{
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    socket = socket_server->nextPendingConnection();
    connect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), this,
            [this](QLocalSocket::LocalSocketError socketError) {
                qDebug() << "stream socket error: " << socketError;
                if (state != Closing) {
                    emit streamError();
                }
            });
    requestHlsManifest();
    requestStream();
}

void
StreamerHls::requestStream()
{
    while (!hls_seg_queue.empty() && state != Closing) {
        QNetworkRequest qnr(QString(hls_seg_queue.dequeue()));
        qnr.setRawHeader(QByteArray("user-agent"), ua);
        auto reply = nam->get(qnr);
        download_buf[downloading_hls_seg].second = false;
        connect(reply, &QNetworkReply::readyRead, this, [this, reply, i = downloading_hls_seg] {
            if (reply->error() != QNetworkReply::NoError) {
                qInfo() << "hls stream error: " << reply->error();
                if (state != Closing) {
                    emit streamError();
                }
                reply->deleteLater();
                return;
            }
            if (state == Idle) {
                state = Running;
                emit streamStart();
                qDebug() << "stream started!";
            }
            download_buf[i].first.append(reply->readAll());
        });
        connect(reply, &QNetworkReply::finished, this, [this, reply, i = downloading_hls_seg] {
            if (reply->error() != QNetworkReply::NoError) {
                qInfo() << "hls stream error: " << reply->error();
                if (state != Closing) {
                    emit streamError();
                }
                reply->deleteLater();
                return;
            }
            download_buf[i].second = true;
            while (download_buf.constBegin() != download_buf.constEnd() && download_buf.constBegin().value().second == true) {
                socket->write(download_buf.constBegin().value().first);
                last_downloaded_hls_seg_a = download_buf.constBegin().key();
                download_buf.remove(download_buf.constBegin().key());
            }
            reply->deleteLater();
            return;
        });
        ++downloading_hls_seg;
    }
    if (state != Closing) {
        QTimer::singleShot(500, this, &StreamerHls::requestStream);
    }
}

void
StreamerHls::requestHlsManifest()
{
    if (last_downloaded_hls_seg_a != last_downloaded_hls_seg_b) {
        no_new_seg_time = 0;
        last_downloaded_hls_seg_b = last_downloaded_hls_seg_a;
    } else {
        ++no_new_seg_time;
    }
    if (no_new_seg_time > 10 && last_downloaded_hls_seg_a != -1) {
        if (state != Closing) {
            emit streamError();
        }
        return;
    }
    QNetworkRequest qnr(real_url);
    qnr.setRawHeader(QByteArray("user-agent"), ua);
    qnr.setTransferTimeout(10000);
    auto reply = nam->get(qnr);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        httpFinished(reply);
    });
    if (state != Closing) {
        QTimer::singleShot(1000, this, &StreamerHls::requestHlsManifest);
    }
}

void
StreamerHls::httpFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "hls manifest error: " << reply->error();
        reply->deleteLater();
        return;
    }
    qint64 base_seq = -1;
    while (reply->canReadLine()) {
        auto line = reply->readLine();
        line.chop(1);
        if (line.startsWith("#EXT-X-MEDIA-SEQUENCE:")) {
            bool ok;
            base_seq = line.mid(22).toLongLong(&ok);
            if (!ok) {
                qDebug() << "hls seq error";
                reply->deleteLater();
                return;
            }
            if (hls_seg == -1) {
                hls_seg = base_seq;
            }
        } else if (line.startsWith("http")) {
            if (base_seq > hls_seg) {
                hls_seg_queue.enqueue(line);
                hls_seg = base_seq++;
            } else {
                ++base_seq;
            }
        }
    }
    reply->deleteLater();
}

StreamerDash::StreamerDash(QString real_url, QString socket_path, QObject* parent)
  : real_url(real_url)
  , stream_socket_path(socket_path)
{
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
    state = Idle;

    auto self = this->sharedFromThis();
    qlp_lib = QSharedPointer<QLivePlayerLib>(new QLivePlayerLib());
    auto fu = QtConcurrent::run(this, &StreamerDash::qlp_run_streamer);
    auto fu1 = QtConcurrent::run(this, &StreamerDash::qlp_check_streamer_loading);
}

void
StreamerDash::close()
{
    state = Closing;
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
