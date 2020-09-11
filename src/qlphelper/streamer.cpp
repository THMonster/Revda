#include <QtGlobal>

#include "streamer.h"

StreamerFlv::StreamerFlv(QString real_url, QString socket_path, QObject *parent)
    : Streamer(parent)
{
    this->real_url = real_url;
    this->stream_socket_path = socket_path;

    proc = new QProcess(this);
    connect(proc, &QProcess::readyReadStandardOutput, this, &StreamerFlv::onProcStdout);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &StreamerFlv::onProcFinished);
}

StreamerFlv::~StreamerFlv()
{
    close();
    QLocalServer::removeServer(stream_socket_path);
}

void StreamerFlv::start()
{
    state = Idle;
    socket_server = new QLocalServer(this);
    connect(socket_server, &QLocalServer::newConnection, this, &StreamerFlv::setSocket);
    socket_server->listen(stream_socket_path);
}

void StreamerFlv::close()
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

void StreamerFlv::setSocket()
{
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    socket = socket_server->nextPendingConnection();
    connect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
            [&](QLocalSocket::LocalSocketError socketError) {
        qDebug() << "stream socket error" << socketError;
        if (state != Closing) {
            emit streamError();
        }
    });
    requestStream();
}

void StreamerFlv::onProcStdout()
{
    if (state == Idle) {
        state = Running;
        emit streamStart();
        qDebug() << "stream started!";
    }
    socket->write(proc->readAllStandardOutput());
}

void StreamerFlv::onProcFinished(int code, QProcess::ExitStatus es)
{
    Q_UNUSED(code);
    Q_UNUSED(es);

    qDebug() << "streamlink exited!!!!";
//    qInfo() << proc->readAllStandardError();
    if (state != Closing) {
        emit streamError();
    }
}

void StreamerFlv::requestStream()
{
    qDebug() << real_url;
    proc->terminate();
    proc->waitForFinished();
    proc->start("curl", QStringList()
                << "-H" << "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.102 Safari/537.36"
                << "-s" << "-L" << real_url);
}

StreamerHls::StreamerHls(QString real_url, QString socket_path, QObject *parent)
    : Streamer(parent)
{
    this->real_url = real_url;
    this->stream_socket_path = socket_path;

    proc = new QProcess(this);
    proc->setReadChannel(QProcess::StandardError);
    connect(proc, &QProcess::readyRead, this, &StreamerHls::onProcStdout);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &StreamerHls::onProcFinished);
}

StreamerHls::~StreamerHls()
{
    close();
    QLocalServer::removeServer(stream_socket_path);
}

void StreamerHls::start()
{
    state = Idle;
    requestStream();
}

void StreamerHls::close()
{
    state = Closing;
    proc->write("q\n");
    proc->terminate();
    proc->waitForFinished();
}

void StreamerHls::onProcStdout()
{
    while (proc->canReadLine()) {
//        qDebug() << proc->readLine();
        if (proc->readLine().contains("Input #0, mpegts")) {
            QTimer::singleShot(2000, [this]() {
                if (state == Idle) {
                    emit this->streamStart();
                    state = Running;
                }
            });
        }
    }
}

void StreamerHls::onProcFinished(int code, QProcess::ExitStatus es)
{
    Q_UNUSED(code);
    Q_UNUSED(es);

    qDebug() << "hls stream finished!!!!";
    if (state != Closing) {
        emit streamError();
    }
}

void StreamerHls::requestStream()
{
    proc->terminate();
    proc->waitForFinished();
//    proc->start("ffmpeg", QStringList() << "-nostats" << "-user_agent"
//                << "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36"
//                << "-i" << real_url << "-c" << "copy" << "-f"
//                << "mpegts" << "-listen" << "1" << "unix://" + stream_socket_path);
    proc->start("sh", QStringList() << "-c"
                << "ffmpeg -nostats -user_agent 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36' -i '"
                + real_url
                + "' -c copy -f mpegts - | ffmpeg -nostats -f mpegts -i - -c copy -f flv -listen 1 unix://"
                + stream_socket_path);
}

StreamerSl::StreamerSl(QString real_url, QString socket_path, QObject *parent)
    : Streamer(parent)
{
    this->real_url = real_url;
    this->stream_socket_path = socket_path;

    proc = new QProcess(this);
    connect(proc, &QProcess::readyReadStandardOutput, this, &StreamerSl::onProcStdout);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &StreamerSl::onProcFinished);
}

StreamerSl::~StreamerSl()
{
    close();
    QLocalServer::removeServer(stream_socket_path);
}

void StreamerSl::start()
{
    state = Idle;
    socket_server = new QLocalServer(this);
    connect(socket_server, &QLocalServer::newConnection, this, &StreamerSl::setSocket);
    socket_server->listen(stream_socket_path);
}

void StreamerSl::close()
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

void StreamerSl::onProcStdout()
{
    if (state == Idle) {
        state = Running;
        emit streamStart();
    }
    socket->write(proc->readAllStandardOutput());
}

void StreamerSl::onProcFinished(int code, QProcess::ExitStatus es)
{
    Q_UNUSED(code);
    Q_UNUSED(es);

    qDebug() << "streamlink exited!!!!";
//    qInfo() << proc->readAllStandardError();
    if (state != Closing) {
        emit streamError();
    }
}

void StreamerSl::requestStream()
{
    proc->terminate();
    proc->waitForFinished();
    proc->start("streamlink", QStringList() << real_url
                << "best" << "--stream-segment-timeout" << "4"
                << "--stream-timeout" << "5" << "--stdout");
//    proc->start("streamlink", QStringList() << real_url
//                << "best" << "-O");
}

void StreamerSl::setSocket()
{
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    socket = socket_server->nextPendingConnection();
    connect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
            [&](QLocalSocket::LocalSocketError socketError) {
        qDebug() << "stream socket error" << socketError;
        if (state != Closing) {
            emit streamError();
        }
    });
    requestStream();
}
