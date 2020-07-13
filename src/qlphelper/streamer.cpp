#include "streamer.h"

Streamer::Streamer(QString room_url, QString stream_socket, QObject *parent)
    : QObject(parent)
{
    this->stream_socket_path = stream_socket;
    this->room_url = room_url;
//    if (room_url.contains("huya.com")) {
//        is_hls = true;
//    }

    nam = new QNetworkAccessManager(this);
    nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    nam->setTransferTimeout(5000);
    ykdl_process = new QProcess(this);

    socket_server = new QLocalServer(this);
    connect(socket_server, &QLocalServer::newConnection, this, &Streamer::setSocket);
    socket_server->listen(stream_socket);
    connect(ykdl_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Streamer::setRealUrl);
    if (is_hls == false) {

    } else {
        hls_ffmpeg_process = new QProcess(this);
        ffmpeg_server_path = QString("/tmp/qlp-%1").arg(QUuid::createUuid().toString());
        socket_hls = new QLocalSocket(this);
        connect(hls_ffmpeg_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [&](int exitCode, QProcess::ExitStatus exitStatus){
            qDebug() << "hls ffmpeg finished!!" << exitCode << exitStatus;
            if (terminate_ffmpeg == false) {
                emit streamError();
            } else {
                terminate_ffmpeg = false;
            }
        });
        connect(socket_hls, &QLocalSocket::readyRead, this, &Streamer::onStreamDataHLS);
    }
}

Streamer::~Streamer()
{
    if  (reply_stream != nullptr) {
        reply_stream->deleteLater();
    }
    QLocalServer::removeServer(stream_socket_path);
    if (is_hls) {
        QLocalServer::removeServer(ffmpeg_server_path);
    }
}


void Streamer::requestStreamHLS()
{
    hls_ffmpeg_process->terminate();
    hls_ffmpeg_process->waitForFinished();
    hls_ffmpeg_process->start("ffmpeg", QStringList() << "-loglevel" << "quiet" << "-user_agent" <<
                              "aaa"
                              << "-i" << real_url << "-c" << "copy" << "-f" << "flv" << "-listen" << "1" << "unix://" + ffmpeg_server_path);
    connectToFFmpeg();
}

void Streamer::onStreamDataHLS()
{
    if (on_streaming == false) {
        on_streaming = true;
        emit streamStart();
    }
    socket->write(socket_hls->readAll());
}

void Streamer::setRealUrl(int code, QProcess::ExitStatus es)
{
    Q_UNUSED(code);
    Q_UNUSED(es);
    QRegularExpression re("^(http.+)$");
    QRegularExpression re_title("^title: +([^\n]+)$");
    while(!ykdl_process->atEnd()) {
        QString line(ykdl_process->readLine());
        QRegularExpressionMatch match = re_title.match(line);
        if (match.hasMatch()) {
             emit titleMatched(match.captured(1));
        }
        match = re.match(line);
        if (match.hasMatch() && real_url.isEmpty()) {
             real_url = match.captured(1);
             offline_counter = 0;
             if (is_hls == false) {
                 requestStream();
             } else {
                 requestStreamHLS();
             }
             return;
        }
    }
    if (real_url.isEmpty()) {
        qCritical() << "Stream url not found!";
        if (offline_counter > 30) {
            QCoreApplication::exit(0);
        } else {
            ++offline_counter;
            QTimer::singleShot(1000, this, &Streamer::requestRealUrl);
        }
    }
}

void Streamer::onStreamData()
{
    if (on_streaming == false) {
        on_streaming = true;
        manual_restart_flag = false;
        emit streamStart();
    }
    socket->write(reply_stream->readAll());
}

void Streamer::onHttpFinished()
{
    qDebug() << "http stream finished!!!!" << reply_stream->errorString();
    if (manual_restart_flag == false) {
        emit streamError();
    } else {
        manual_restart_flag = false;
    }
    if (reply_stream != nullptr) {
        reply_stream->deleteLater();
        reply_stream = nullptr;
    }
}


void Streamer::start()
{
    requestRealUrl();
}

void Streamer::restart()
{
    if (is_hls) {
        if (hls_ffmpeg_process->state() != QProcess::NotRunning) {
            terminate_ffmpeg = true;
        }
        socket_hls->disconnectFromServer();
        hls_ffmpeg_process->terminate();
    }
    if (reply_stream != nullptr) {
        manual_restart_flag = true;
        reply_stream->abort();
        reply_stream->deleteLater();
        reply_stream = nullptr;
    }
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    on_streaming = false;
    real_url = "";
    requestRealUrl();
}

void Streamer::stop()
{
    if (is_hls) {
        if (hls_ffmpeg_process->state() != QProcess::NotRunning) {
            terminate_ffmpeg = true;
        }
        socket_hls->disconnectFromServer();
        hls_ffmpeg_process->terminate();
    }
    if (reply_stream != nullptr) {
        reply_stream->abort();
        reply_stream->deleteLater();
        reply_stream = nullptr;
    }
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    on_streaming = false;
    real_url = "";
}

void Streamer::setSocket()
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
        emit streamError();
    });
}

void Streamer::connectToFFmpeg()
{
    if (socket_hls->state() == QLocalSocket::UnconnectedState) {
        socket_hls->connectToServer(ffmpeg_server_path);
        QTimer::singleShot(1000, this, &Streamer::connectToFFmpeg);
    }
}

void Streamer::requestStream()
{
    qDebug() << real_url;
    QNetworkRequest qnr(real_url);
    qnr.setMaximumRedirectsAllowed(3);
    qnr.setTransferTimeout(5000);
    qnr.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36");
    reply_stream = nam->get(qnr);
    connect(reply_stream, &QNetworkReply::readyRead, this, &Streamer::onStreamData);
    connect(reply_stream, &QNetworkReply::finished, this, &Streamer::onHttpFinished);
}

void Streamer::requestRealUrl()
{
    qDebug() << "start ykdl!";
    ykdl_process->waitForFinished(10000);
    ykdl_process->start("ykdl", QStringList() << "-i" << room_url);
}
