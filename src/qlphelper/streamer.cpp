#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>

#include "streamer.h"

#define qsl(s) QStringLiteral(s)

StreamerFlv::StreamerFlv(QString real_url, QString room_url, QString socket_path, QObject* parent)
  : Streamer(parent)
  , real_url(real_url)
  , room_url(room_url)
  , stream_socket_path(socket_path)
{
    proc = new QProcess(this);
    connect(proc, &QProcess::readyReadStandardOutput, this, &StreamerFlv::onProcStdout);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &StreamerFlv::onProcFinished);
}

StreamerFlv::~StreamerFlv()
{
    close();
    QLocalServer::removeServer(stream_socket_path);
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
  : Streamer(parent)
  , real_url(real_url)
  , stream_socket_path(socket_path)
  , ua("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36")
{
    nam = new QNetworkAccessManager(this);
    nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

StreamerHls::~StreamerHls()
{
    close();
    QLocalServer::removeServer(stream_socket_path);
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
  : Streamer(parent)
  , stream_socket_path(socket_path)
  , user_agent("curl/7.78.0")
  , referer("https://www.youtube.com")
{
    auto u = real_url.split("\n", Qt::SkipEmptyParts);
    if (u.length() == 3) {
        this->real_url_v = u[0];
        this->real_url_a = u[1];
        this->downloading_dash_seg = u[2].toLongLong();
    }
    nam = new QNetworkAccessManager(this);
    nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    initFifo();
}

StreamerDash::~StreamerDash()
{
    close();
    fifo_v->close();
    fifo_v->remove();
    fifo_a->close();
    fifo_a->remove();
    //    QLocalServer::removeServer(stream_socket_path + qsl("-v"));
    //    QLocalServer::removeServer(stream_socket_path + qsl("-a"));
}

void
StreamerDash::start()
{
    state = Idle;

    //    socket_server_v = new QLocalServer(this);
    //    connect(socket_server_v, &QLocalServer::newConnection, this, &StreamerDash::setSocketV);
    //    socket_server_v->listen(stream_socket_path + qsl("-v"));
    //    socket_server_a = new QLocalServer(this);
    //    connect(socket_server_a, &QLocalServer::newConnection, this, &StreamerDash::setSocketA);
    //    socket_server_a->listen(stream_socket_path + qsl("-a"));

    push_timer = new QTimer(this);
    connect(push_timer, &QTimer::timeout, this, &StreamerDash::pushStream);
    QTimer::singleShot(500, this, &StreamerDash::requestStream);
    this->push_timer->start(1000);
}

void
StreamerDash::close()
{
    state = Closing;
    if (socket_v != nullptr) {
        socket_v->abort();
        socket_v->deleteLater();
        socket_v = nullptr;
    }
    if (socket_a != nullptr) {
        socket_a->abort();
        socket_a->deleteLater();
        socket_a = nullptr;
    }
    if (fifo_v_proc != nullptr) {
        fifo_v_proc->terminate();
        fifo_v_proc->waitForFinished(2000);
    }
    if (fifo_a_proc != nullptr) {
        fifo_a_proc->terminate();
        fifo_a_proc->waitForFinished(2000);
    }
}

void
StreamerDash::setSocketV()
{
    if (socket_v != nullptr) {
        socket_v->deleteLater();
        socket_v = nullptr;
    }
    socket_v = socket_server_v->nextPendingConnection();
    connect(socket_v, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), this,
            [this](QLocalSocket::LocalSocketError socketError) {
                qDebug() << "stream socket error" << socketError;
                setError();
            });
}

void
StreamerDash::setSocketA()
{
    if (socket_a != nullptr) {
        socket_a->deleteLater();
        socket_a = nullptr;
    }
    socket_a = socket_server_a->nextPendingConnection();
    connect(socket_a, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), this,
            [this](QLocalSocket::LocalSocketError socketError) {
                qDebug() << "stream socket error" << socketError;
                setError();
            });
}

void
StreamerDash::requestStream()
{
    if (download_threads_num > 0) {
        --download_threads_num;
    } else {
        return;
    }
    QString uv, ua;
    uv = this->real_url_v + qsl("&sq=%1").arg(downloading_dash_seg) + qsl("&rn=%1").arg(yt_dash_rn);
    ua = this->real_url_a + qsl("&sq=%1").arg(downloading_dash_seg) + qsl("&rn=%1").arg(yt_dash_rn);
    QNetworkRequest qnrv(uv);
    QNetworkRequest qnra(ua);
    qnrv.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    qnra.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    qnrv.setRawHeader(QByteArray("User-Agent"), user_agent);
    qnra.setRawHeader(QByteArray("User-Agent"), user_agent);
    qnrv.setRawHeader(QByteArray("Accept"), QByteArray("*/*"));
    qnra.setRawHeader(QByteArray("Accept"), QByteArray("*/*"));
    //    qnrv.setRawHeader(QByteArray("Referer"), referer);
    //    qnra.setRawHeader(QByteArray("Referer"), referer);
    auto reply_v = nam->get(qnrv);
    auto reply_a = nam->get(qnra);
    //    qInfo().noquote() << uv << ua;
    download_status[downloading_dash_seg] = 0;
    video_buf[downloading_dash_seg].clear();
    audio_buf[downloading_dash_seg].clear();
    connect(reply_v, &QNetworkReply::readyRead, this, [this, reply_v, i = downloading_dash_seg] {
        if (reply_v->error() != QNetworkReply::NoError) {
            qInfo() << "dash stream error: " << reply_v->error();
            setError();
            reply_v->deleteLater();
            return;
        }
        video_buf[i].append(reply_v->readAll());
    });
    connect(reply_v, &QNetworkReply::finished, this, [this, reply_v, i = downloading_dash_seg] {
        if (reply_v->error() != QNetworkReply::NoError) {
            qInfo() << "dash stream error: " << reply_v->error();
            setError();
            reply_v->deleteLater();
            return;
        }
        download_status[i] |= 1;
        if ((download_status[i] & 3) == 3) {
            this->download_threads_num++;
            this->requestStream();
        }
        reply_v->deleteLater();
    });
    connect(reply_a, &QNetworkReply::readyRead, this, [this, reply_a, i = downloading_dash_seg] {
        if (reply_a->error() != QNetworkReply::NoError) {
            qInfo() << "dash stream error: " << reply_a->error();
            setError();
            reply_a->deleteLater();
            return;
        }
        audio_buf[i].append(reply_a->readAll());
    });
    connect(reply_a, &QNetworkReply::finished, this, [this, reply_a, i = downloading_dash_seg] {
        if (reply_a->error() != QNetworkReply::NoError) {
            qInfo() << "dash stream error: " << reply_a->error();
            setError();
            reply_a->deleteLater();
            return;
        }
        download_status[i] |= 2;
        if ((download_status[i] & 3) == 3) {
            this->download_threads_num++;
            this->requestStream();
        }
        reply_a->deleteLater();
    });
    ++downloading_dash_seg;
    ++yt_dash_rn;
    requestStream();
}

void
StreamerDash::initFifo()
{
    QProcess::execute("mkfifo", QStringList() << stream_socket_path + "-v");
    QProcess::execute("mkfifo", QStringList() << stream_socket_path + "-a");
    fifo_v = new QFile(stream_socket_path + "-v", this);
    fifo_a = new QFile(stream_socket_path + "-a", this);
    fifo_v_proc = new QProcess(this);
    fifo_a_proc = new QProcess(this);
    connect(fifo_v_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitStatus);
        setError();
    });
    connect(fifo_a_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitStatus);
        setError();
    });
    fifo_v_proc->start("dd", QStringList() << "of=" + stream_socket_path + "-v"
                                           << "status=none");
    fifo_a_proc->start("dd", QStringList() << "of=" + stream_socket_path + "-a"
                                           << "status=none");
}

void
StreamerDash::pushStream()
{
    no_new_seg_time++;
    //    qInfo() << download_status;
    while (video_buf.constBegin() != video_buf.constEnd()) {
        auto seg = video_buf.constBegin().key();
        if (((download_status[seg] & 7) == 3)) {
            if (state == Idle) {
                state = Running;
                emit streamStart();
                qDebug() << "stream started!";
            }
            if (fifo_v_proc->write(video_buf.constBegin().value()) == -1) {
                setError();
                return;
            }
            video_buf.remove(seg);
            download_status[seg] |= 4;
        } else {
            break;
        }
    }
    while (audio_buf.constBegin() != audio_buf.constEnd()) {
        auto seg = audio_buf.constBegin().key();
        if (((download_status[seg] & 11) == 3)) {
            if (fifo_a_proc->write(audio_buf.constBegin().value()) == -1) {
                setError();
                return;
            }
            audio_buf.remove(seg);
            download_status[seg] |= 8;
        } else {
            break;
        }
    }

    while (download_status.constBegin() != download_status.constEnd() && download_status.constBegin().value() == 15) {
        download_status.remove(download_status.constBegin().key());
        //        qInfo() << "remove loaded";
        no_new_seg_time = 0;
    }

    if (no_new_seg_time > 15) {
        qInfo() << "stream timeout, reloading...";
        setError();
    }
}

void
StreamerDash::setError()
{
    if (state != Closing) {
        qInfo() << "set error";
        emit streamError();
        state = Closing;
    }
}
