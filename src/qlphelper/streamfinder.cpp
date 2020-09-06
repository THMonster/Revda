#include "streamfinder.h"

StreamFinder::StreamFinder(QString room_url, QString stream_socket, QObject *parent) : QObject(parent)
{
    this->room_url = room_url;
    this->stream_socket = stream_socket;

    proc = new QProcess(this);

    if (room_url.contains("youtube.com/c")) {
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &StreamFinder::slotYtbResponse);
    } else {
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &StreamFinder::slotYkdlResponse);
    }
}

StreamFinder::~StreamFinder()
{
    if (streamer != nullptr) {
        streamer->close();
        streamer->deleteLater();
        streamer = nullptr;
    }
    proc->terminate();
    proc->waitForFinished(3000);
}

void StreamFinder::start()
{
    startRequest();
}

void StreamFinder::restart()
{
    real_url.clear();
    if (streamer != nullptr) {
        streamer->close();
        streamer->deleteLater();
        streamer = nullptr;
    }
    startRequest();
}

void StreamFinder::stop()
{
    real_url.clear();
    if (streamer != nullptr) {
        streamer->close();
        streamer->deleteLater();
        streamer = nullptr;
    }
}

void StreamFinder::startRequest()
{
    qInfo() << "Finding stream...";
    proc->waitForFinished(10000);
    auto tid = ++proc_id;
    if (room_url.contains("youtube.com/c")) {
        QStringList args;
        args.append(QStandardPaths::locate(QStandardPaths::DataLocation, "yt_url.py"));
        args.append(room_url);
        proc->start("python", args);
        QTimer::singleShot(20000, [this, tid]() {
            if (this->proc->state() == QProcess::Running && tid == this->proc_id) {
                this->proc->kill();
            }
        });
    } else {
        proc->start("ykdl", QStringList() << "-i" << room_url);
        QTimer::singleShot(10000, [this, tid]() {
            if (this->proc->state() == QProcess::Running && tid == this->proc_id) {
                this->proc->kill();
            }
        });
    }
}

void StreamFinder::slotYtbResponse(int code, QProcess::ExitStatus es)
{
    Q_UNUSED(code);
    Q_UNUSED(es);
    real_url.clear();
    QRegularExpression re("^(http.+)$");
    QRegularExpression re_title("^title: +([^\n]+)$");
    while(!proc->atEnd()) {
        QString line(proc->readLine());
        QRegularExpressionMatch match = re_title.match(line);
        if (match.hasMatch()) {
             emit titleMatched(match.captured(1));
        }
        match = re.match(line);
        if (match.hasMatch() && real_url.isEmpty()) {
             real_url = match.captured(1);
             offline_counter = 0;
        }
    }
    startStreamer();
}

void StreamFinder::slotYkdlResponse(int code, QProcess::ExitStatus es)
{
    Q_UNUSED(code);
    Q_UNUSED(es);
    real_url.clear();
    QRegularExpression re("^(http.+)$");
    QRegularExpression re_title("^title: +([^\n]+)$");
    while(!proc->atEnd()) {
        QString line(proc->readLine());
        QRegularExpressionMatch match = re_title.match(line);
        if (match.hasMatch()) {
             emit titleMatched(match.captured(1));
        }
        match = re.match(line);
        if (match.hasMatch() && real_url.isEmpty()) {
             real_url = match.captured(1);
             offline_counter = 0;
        }
    }
    startStreamer();
}

void StreamFinder::startStreamer()
{
    if (real_url.isEmpty()) {
        qCritical() << "Stream url not found!";
        if (offline_counter > 30) {
            QCoreApplication::exit(0);
        } else {
            ++offline_counter;
            QTimer::singleShot(1000, this, &StreamFinder::startRequest);
        }
    } else {
        if (real_url.right(7) == ":qlp-sl") {
            real_url.chop(7);
            streamer = new StreamerSl(real_url, stream_socket, this);
            connect(streamer, &Streamer::streamError, [this]() {
                emit this->streamError();
            });
            connect(streamer, &Streamer::streamStart, [this]() {
                emit this->ready();
                emit this->streamStart();
            });
            streamer->start();
        } else if (real_url.contains(".m3u8")) {
            streamer = new StreamerHls(real_url, stream_socket, this);
            connect(streamer, &Streamer::streamError, [this]() {
                emit this->streamError();
            });
            connect(streamer, &Streamer::streamStart, [this]() {
                emit this->ready();
                emit this->streamStart();
            });
            streamer->start();
        } else {
            streamer = new StreamerFlv(real_url, stream_socket, this);
            connect(streamer, &Streamer::streamError, [this]() {
                emit this->streamError();
            });
            connect(streamer, &Streamer::streamStart, [this]() {
                emit this->streamStart();
            });
            streamer->start();
            emit ready();
        }
    }
}
