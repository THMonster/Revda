#include "streamfinder.h"

StreamFinder::StreamFinder(QString room_url, QString stream_socket, QObject* parent)
  : QObject(parent)
{
    this->room_url = room_url;
    this->stream_socket = stream_socket;

    proc = new QProcess(this);

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &StreamFinder::slotSfpyResponse);
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
    proc->waitForFinished(10000);
    auto tid = ++proc_id;
    QStringList args;
    args.append(QStandardPaths::locate(QStandardPaths::AppDataLocation, "streamfinder.pyz"));
    args.append(room_url);
    if (QFileInfo::exists(QStringLiteral("/tmp/unlock-qlp"))) {
        if (room_url.contains(QStringLiteral("live.bilibili.com/"))) {
            QSettings s("QLivePlayer", "QLivePlayer", this);
            args.append(s.value("bcookie", QString("")).toString());
        }
    }
    proc->start("python3", args);
    QTimer::singleShot(20000, [this, tid]() {
        if (this->proc->state() == QProcess::Running && tid == this->proc_id) {
            this->proc->kill();
        }
    });
}

void
StreamFinder::slotSfpyResponse(int code, QProcess::ExitStatus es)
{
    Q_UNUSED(code);
    Q_UNUSED(es);
    real_url.clear();
    QRegularExpression re("^(http.+)$");
    QRegularExpression re_title("^title: +([^\n]+)$");
    while (!proc->atEnd()) {
        QString line(proc->readLine());
        QRegularExpressionMatch match = re_title.match(line);
        if (match.hasMatch()) {
            title = match.captured(1);
            emit titleMatched(title);
        }
        match = re.match(line);
        if (match.hasMatch() && real_url.isEmpty()) {
            real_url = match.captured(1);
            offline_counter = 0;
        }
    }
    startStreamer();
}

void
StreamFinder::startStreamer()
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
        if (real_url.right(5) == "::hls") {
            real_url.chop(5);
            streamer = new StreamerSl(real_url, stream_socket, quality, this);
            connect(streamer, &Streamer::streamError, [this]() { emit this->streamError(); });
            connect(streamer, &Streamer::streamStart, [this]() { emit this->streamStart(); });
            streamer->start();
            emit ready(title, 1);
        } else if (real_url.contains(".m3u8")) {
            streamer = new StreamerSl("hls://" + real_url, stream_socket, quality, this);
            connect(streamer, &Streamer::streamError, [this]() { emit this->streamError(); });
            connect(streamer, &Streamer::streamStart, [this]() { emit this->streamStart(); });
            streamer->start();
            emit this->ready(title, 1);
        } else {
            streamer = new StreamerFlv(real_url, stream_socket, this);
            connect(streamer, &Streamer::streamError, [this]() { emit this->streamError(); });
            connect(streamer, &Streamer::streamStart, [this]() { emit this->streamStart(); });
            streamer->start();
            emit ready(title, 0);
        }
    }
}
