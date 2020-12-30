#include <iostream>
#include <QStringBuilder>
#include "bilivideo.h"
#include "../qlpconfig.h"
using namespace BV;

MpvControl::MpvControl(QObject *parent) : QObject(parent)
{
    mpv_socket_path = QString("/tmp/qlp-%1").arg(QUuid::createUuid().toString());
    mpv_proc = new QProcess(this);
    mpv_socket = new QLocalSocket(this);
    connect(mpv_socket, &QLocalSocket::readyRead, this, &MpvControl::readMpvSocket);
    connect(mpv_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus){
        Q_UNUSED(exitStatus);
        QCoreApplication::exit(exitCode);
    });

}

MpvControl::~MpvControl()
{
    mpv_proc->terminate();
    mpv_proc->waitForFinished(3000);
    QLocalServer::removeServer(mpv_socket_path);
}

void MpvControl::start()
{
    QStringList args;
    args.append("--user-agent=Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36");
    args.append("--http-header-fields-add=Referer: https://www.bilibili.com/");
    mpv_proc->start("mpv", args << "--idle=yes" << "--player-operation-mode=pseudo-gui"
                    << "--vf=lavfi=\"fps=60\"" << "--input-ipc-server=" + mpv_socket_path);
    auto t = new QTimer(this);
    connect(t, &QTimer::timeout, [&, t]() {
        if (mpv_socket->state() != QLocalSocket::ConnectedState) {
            mpv_socket->connectToServer(mpv_socket_path);
        } else {
            mpv_socket->write(QString("{ \"command\": [\"keybind\", \"alt+r\", \"script-message qlp:r\"] }\n"
                                      "{ \"command\": [\"keybind\", \"alt+z\", \"script-message qlp:fsdown\"] }\n"
                                      "{ \"command\": [\"keybind\", \"alt+x\", \"script-message qlp:fsup\"] }\n"
                                      "{ \"command\": [\"keybind\", \"alt+b\", \"script-message qlp:back\"] }\n"
                                      "{ \"command\": [\"keybind\", \"alt+n\", \"script-message qlp:next\"] }\n").toUtf8());
            t->stop();
            t->deleteLater();
        }
    });
    t->start(1000);
}

void MpvControl::loadVideo(QString edl_url, QString ass_file_path, QString title)
{
    ass_path = ass_file_path;
    // do loadfile
    mpv_socket->write(QString("{ \"command\": [\"loadfile\", \"%1\"], \"async\": true }\n").arg(edl_url).toUtf8());
    mpv_socket->write(QString("{ \"command\": [\"set_property\", \"force-media-title\", \"%1\"] }\n").arg(title).toUtf8());
}

void MpvControl::readMpvSocket()
{
    while (mpv_socket->canReadLine()) {
        auto tmp = mpv_socket->readLine().trimmed();
        //        qDebug() << tmp;
        if (tmp.contains("qlp:")) {
            auto jobj = QJsonDocument::fromJson(tmp).object();
            auto parser = QlpCmdParser(jobj["args"].toArray()[0].toString());
            if (parser.getReload()) {
                emit requestReload();
            }
            if (parser.getPage() != -1) {
                emit jumpReceived(parser.getPage());
            }
            if (parser.getPageNext()) {
                emit nextReceived();
            }
            if (parser.getPageBack()) {
                emit prevReceived();
            }
            if (parser.getFs() != -1) {
                emit onFont(parser.getFs(), -1);
            }
            if (parser.getFa() != -1) {
                emit onFont(-1, parser.getFa());
            }
            if (parser.getFsUp()) {
                emit onFontScaleDelta(0.15);
            }
            if (parser.getFsDown()) {
                emit onFontScaleDelta(-0.15);
            }
        } else if (tmp.contains("end-file")) {
            emit playFinished();
        } else if (tmp.contains("file-loaded")) {
            emit fileLoaded();
            mpv_socket->write(QString("{ \"command\": [\"sub-add\", \"%1\"], \"async\": true }\n").arg(ass_path).toUtf8());
        }
    }
}

BiliVideo::BiliVideo(QStringList args, QObject *parent)
    : QObject(parent)
{
    ass_file = new QFile(QString("/tmp/qlp-%1.ass").arg(QUuid::createUuid().toString()), this);

    nam = new QNetworkAccessManager(this);
//    nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    QSettings s("QLivePlayer", "QLivePlayer", this);
    cookie = s.value("bcookie", QString("")).toString();
    hevc = s.value("bhevc", false).toBool();
    bool ok = false;
    auto fs = args.at(0).toDouble(&ok);
    if (ok == false || fs <= 0) {
        fs = QlpConfig::getInstance().readFontScale();
    }
    auto fa = args.at(1).toDouble(&ok);
    if (ok == false || (fa < 0 || fa > 1)) {
        fa = QlpConfig::getInstance().readFontAlpha();
    }
    font_size = 40 * fs;

    mpv = new MpvControl(this);
    connect(mpv, &MpvControl::jumpReceived, this, &BiliVideo::playPage);
    connect(mpv, &MpvControl::requestReload, [this]() {
        this->playPage(this->current_page);
    });
    connect(mpv, &MpvControl::prevReceived, this, &BiliVideo::goPrevPage);
    connect(mpv, &MpvControl::nextReceived, this, &BiliVideo::goNextPage);
//    connect(mpv, &MpvControl::playFinished, this, &BiliVideo::autoNextPage);
    connect(mpv, &MpvControl::onFont, this, &BiliVideo::setFont);
    connect(mpv, &MpvControl::onFontScaleDelta, this, &BiliVideo::setFontScaleDelta);
    connect(mpv, &MpvControl::fileLoaded, [this]() {
        connect(this->mpv, &MpvControl::playFinished, this, &BiliVideo::autoNextPage);
    });
}

BiliVideo::~BiliVideo()
{
    ass_file->remove();
    for (const auto& f : seg_file_list) {
        f->remove();
    }
    if (merge_file) {
        merge_file->remove();
    }
}

void BiliVideo::run(QString url)
{
    if (!saved_file) {
        mpv->start();
    }
    QUrl qu(url);
    QUrlQuery quq(qu);
    if (quq.hasQueryItem("p")) {
        current_page = quq.queryItemValue("p").toInt();
        if (current_page == 0) {
            current_page = 1;
        }
    }
    base_url = qu.adjusted(QUrl::RemoveQuery).toString();
    QNetworkRequest qnr(base_url);
    qnr.setMaximumRedirectsAllowed(5);
    qnr.setRawHeader(QByteArray("User-Agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36"));
    qnr.setRawHeader(QByteArray("Referer"), QByteArray("https://www.bilibili.com/"));
    reply_info = nam->get(qnr);
    connect(reply_info, &QNetworkReply::finished, this, &BiliVideo::slotHttpVideoInfo);
}

void BiliVideo::setFont(double fs, double fa)
{
    if (fs > 0) {
        font_size = 40 * fs;
    }
    if (fa >= 0 && fa <= 1) {
//        font_alpha = QStringLiteral("%1").arg((uint)(255*fa), 2, 16, QLatin1Char('0'));
    }
}

void BiliVideo::setFontScaleDelta(double delta)
{
    font_size = font_size + (40 * delta);
}

void BiliVideo::genAss()
{
    if (!ass_file->open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(ass_file);
    out << "[Script Info]\n"
           "; Script generated by QLivePlayer\n"
           "; https://github.com/IsoaSFlus/QLivePlayer\n"
           "Title: Danmaku file\n"
           "ScriptType: v4.00+\n"
           "WrapStyle: 0\n"
           "ScaledBorderAndShadow: yes\n"
           "YCbCr Matrix: None\n"
           + QStringLiteral("PlayResX: %1\n").arg(QString::number(res_x)) +
           "PlayResY: 1080\n"
           "[V4+ Styles]\n"
           "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n"
           "Style: Default,Sans,"
           + QString::number(font_size) +
           ",&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,1,0,7,0,0,0,1\n"
           "[Events]\n"
           "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n";
    for (int i = 0; i < 30; i++) {
        danmaku_channel[i].length = 1;
        danmaku_channel[i].begin_pts = -20;
    }
    auto iter = danmaku_map.constBegin();
    int display_length = 0;
    int avail_channel = -1;
    QString t1 ,t2, c;
    while (iter != danmaku_map.constEnd()) {
//        qDebug() << iter.key() << iter.value();
        if (iter.value().first[0] == '4') {
            avail_channel = 1; // bottom
        } else if (iter.value().first[0] == '5') {
            avail_channel = 1; // top
        } else {
            display_length = getDankamuDisplayLength(iter.value().first.mid(1), font_size);
            avail_channel = getAvailDMChannel(iter.key(), display_length);
        }
        if (avail_channel >= 0) {
            t1 = QTime::fromMSecsSinceStartOfDay(iter.key()*1000.0).toString("hh:mm:ss.zzz");
            t2 = QTime::fromMSecsSinceStartOfDay((iter.key()+speed)*1000.0).toString("hh:mm:ss.zzz");
            t1.chop(1);
            t2.chop(1);
            c = QString::number(iter.value().second, 16);
            QStringView sv1{c};
            auto sv2 = sv1.mid(4, 2) % sv1.mid(2, 2) % sv1.mid(0, 2);
            if (iter.value().first[0] == '4') {
                out << QString("Dialogue: 0,%2,%3,Default,,0,0,0,,{\\1c&%4&\\an2}%1")
                       .arg(iter.value().first.mid(1), t1, t2, sv2) << "\n";
            } else if (iter.value().first[0] == '5') {
                out << QString("Dialogue: 0,%2,%3,Default,,0,0,0,,{\\1c&%4&\\an8}%1")
                            .arg(iter.value().first.mid(1), t1, t2, sv2) << "\n";
            } else {
                out << QString("Dialogue: 0,%4,%5,Default,,0,0,0,,{\\1c&%6&\\move(%7,%1,%2,%1)}%3")
                            .arg(QString::number(avail_channel*(font_size)),
                                 QString::number(0-display_length), iter.value().first.mid(1),
                                 t1, t2, sv2, QString::number(res_x)) << "\n";
            }
        }
        ++iter;
    }
    ass_file->close();
    if (saved_file) {
        downloadVideo();
    } else {
        mpv->loadVideo(edl_url, ass_file->fileName(), title);
    }
}

void BiliVideo::setRes()
{
    QProcess p;
    p.start("ffprobe", QStringList()
            << "-show_streams" << "-loglevel" << "quiet" << "-select_streams" << "v"
            << "-user_agent" << "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36"
            << "-headers" << "Referer: https://www.bilibili.com/" << real_url[0]);
    p.waitForStarted(5000);
    p.waitForFinished();
    QRegularExpression reh("\\nheight=([0-9]+)");
    QRegularExpression rew("\\nwidth=([0-9]+)");
    auto a = p.readAll();
    auto matchh = reh.match(a);
    auto matchw = rew.match(a);
//    qInfo() << matchw.captured(1).toDouble() << matchh.captured(1).toDouble() << a;
    if (matchw.hasMatch() && matchh.hasMatch()) {
         res_x = res_y * matchw.captured(1).toDouble() / matchh.captured(1).toDouble();
    }
}

int BiliVideo::getDankamuDisplayLength(QString dm, int fontsize)
{
    int ascii_num = 0;
    int dm_size = dm.size();
    if (dm_size <= 0) {
        return fontsize * 0.75 * 10;
    }
    const QChar *data = dm.constData();
    while (dm_size > 0) {
        if (data->unicode() < 128) {
            ++ascii_num;
        }
        ++data;
        --dm_size;
    }
    return (fontsize * 0.75 * dm.size()) - (fontsize * 0.25 * ascii_num);
}

int BiliVideo::getAvailDMChannel(double time_start, int len)
{
    double s = (res_x + (double)len) / this->speed;
    for (int i = 0; i < 20; i++)
    {
        if (((this->speed - time_start + danmaku_channel[i].begin_pts) * s) > res_x) {
            continue;
        } else {
            if ((((double)danmaku_channel[i].length + res_x) * (time_start - danmaku_channel[i].begin_pts) / this->speed) < danmaku_channel[i].length ) {
                continue;
            } else {
                danmaku_channel[i].length = len;
                danmaku_channel[i].begin_pts = time_start;
                return i;
            }
        }
    }
    return -4;
}

void BiliVideo::slotHttpDMXml()
{
    if (reply_dm->error() != QNetworkReply::NoError) {
        qDebug() << "Network error: " << reply_dm->error();
        reply_dm->deleteLater();
        return;
    }
    danmaku_map.clear();
    QString xml(reply_dm->readAll());
    int cur = -1;
    int i = -1, j = -1;
    while (1) {
        cur = xml.indexOf("<d p=\"", cur + 1);
        if (cur == -1) {
            break;
        }
        i = xml.indexOf("\"", cur + 6);
        j = xml.indexOf("</d>", cur);
        danmaku_map.insert(QStringView{xml}.mid(cur + 6, i - cur - 6).split(',', Qt::SkipEmptyParts)[0].toDouble(),
                QPair<QString, int>(QStringView{xml}.mid(cur + 6, i - cur - 6).split(',', Qt::SkipEmptyParts)[1] % QStringView{xml}.mid(i + 2, j - i - 2), QStringView{xml}.mid(cur + 6, i - cur - 6).split(',', Qt::SkipEmptyParts)[3].toInt()));
    }

    genAss();
}

void BiliVideo::slotHttpVideoInfo()
{
    if (reply_info->error() != QNetworkReply::NoError) {
        qDebug() << "Network error: " << reply_info->error();
        reply_info->deleteLater();
        return;
    }
    pages.clear();
//    qInfo() << reply_info->readAll();
    QRegularExpression re("__INITIAL_STATE__=({.+?});");
    auto m = re.match(reply_info->readAll());
    if (m.hasMatch()) {
//        qInfo() << m.captured(1);
        auto jobj = QJsonDocument::fromJson(m.captured(1).toUtf8()).object();
        if (jobj.find("epList") != jobj.end()) {
            if (current_page == 1) {
                current_page = jobj.value("epInfo").toObject().value("i").toInt(0) + 1;
                if (current_page == 0) {
                    current_page = 1;
                }
            }
            auto arr = jobj.value("epList").toArray();
            if (!arr.isEmpty()) {
                for (const auto& p : arr) {
                    pages.append("ep" + QString::number(p.toObject().value("id").toInt()));
                }
            }
        } else if (jobj.find("videoData") != jobj.end()) {
            auto videos = jobj.value("videoData").toObject().value("videos").toInt(1);
            while (videos > 0) {
                pages.append("");
                videos--;
            }
        } else {

        }
    }
    if (!pages.isEmpty()) {
        playPage(current_page);
    }
    qDebug() << pages << current_page;
}

inline void BiliVideo::genEDLUrl()
{
    if (real_url[0].contains(".m4s")) {
        if (real_url.length() == 3 && hevc == true) {
            edl_url = QString("edl://!no_clip;!no_chapters;\%%1\%%3;!new_stream;!no_clip;!no_chapters;\%%2\%%4")
                    .arg(real_url[1].length()).arg(real_url[2].length()).arg(real_url[1]).arg(real_url[2]);
        } else {
            edl_url = QString("edl://!no_clip;!no_chapters;\%%1\%%3;!new_stream;!no_clip;!no_chapters;\%%2\%%4")
                    .arg(real_url[1].length()).arg(real_url[0].length()).arg(real_url[1]).arg(real_url[0]);
        }
    } else {
        edl_url = QString("edl://");
        for (const auto& u : real_url) {
            edl_url.append(QString("\%%1\%%2;").arg(u.length()).arg(u));
        }
    }
}

void BiliVideo::downloadVideo()
{
    QProcess *p = new QProcess(this);
    connect(p, &QProcess::readyReadStandardError, [=] () {
        std::cerr << p->readAllStandardError().toStdString();
    });
    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus){
        Q_UNUSED(exitStatus);
        QCoreApplication::exit(exitCode);
    });
    if (real_url[0].contains(".m4s")) {
        if (real_url.length() == 3 && hevc == true) {
            p->start("ffmpeg", QStringList()
                    << "-user_agent" << "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36"
                    << "-headers" << "Referer: https://www.bilibili.com/" << "-i" << real_url[2]
                    << "-user_agent" << "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36"
                    << "-headers" << "Referer: https://www.bilibili.com/" << "-i" << real_url[1]
                    << "-i" << ass_file->fileName()
                    << "-c" << "copy" << saved_file->fileName());
        } else {
            p->start("ffmpeg", QStringList()
                    << "-user_agent" << "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36"
                    << "-headers" << "Referer: https://www.bilibili.com/" << "-i" << real_url[0]
                    << "-user_agent" << "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36"
                    << "-headers" << "Referer: https://www.bilibili.com/" << "-i"<< real_url[1]
                    << "-i" << ass_file->fileName()
                    << "-c" << "copy" << saved_file->fileName());
        }
    } else {
        startSegFifoProc();
        this->merge_file = new QFile(QString("/tmp/qlp-%1.txt").arg(QUuid::createUuid().toString()), this);
        if (!this->merge_file->open(QIODevice::WriteOnly | QIODevice::Text))
            return;
        QTextStream out(this->merge_file);
        for (const auto& f : this->seg_file_list) {
            out << "file ";
            out << f->fileName();
            out << "\n";
        }
        this->merge_file->close();
        p->start("ffmpeg", QStringList()
                 << "-f" << "concat" << "-safe" << "0"
                 << "-i" << this->merge_file->fileName()
                 << "-i" << this->ass_file->fileName()
                 << "-c" << "copy" << this->saved_file->fileName());
    }
}

void BiliVideo::setSavedFilePath(QString path)
{
    this->saved_file = new QFile(path, this);
}

void BiliVideo::startSegFifoProc()
{
    for (const auto& u : real_url) {
        QProcess *p = new QProcess(this);
        QFile *f = new QFile(QString("/tmp/qlp-%1.flv").arg(QUuid::createUuid().toString()), this);
        seg_file_list.append(f);
        QProcess::execute("mkfifo", QStringList() << seg_file_list.last()->fileName());
        p->start("curl", QStringList()
                 << u
                 << "-H" << "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36"
                 << "-H" << "Referer: https://www.bilibili.com/"
                 << "-o" << seg_file_list.last()->fileName());
    }
}

void BiliVideo::requestRealUrl(QString url)
{
    real_url.clear();
    title.clear();
    res_x = 1920;
    res_y = 1080;
    QProcess p;
    QStringList args;
    args.append(QStandardPaths::locate(QStandardPaths::AppDataLocation, "streamfinder.pyz"));
    args.append(url);
    args.append(cookie);
    qDebug() << args;
    p.start("python3", args);
    p.waitForStarted(5000);
    p.waitForFinished();
    QRegularExpression re("^(http.+)$");
    while(!p.atEnd()) {
        QString line(p.readLine());
        qDebug() << line;
        if (line.left(6) == "title:") {
            line.remove(0, 6);
            line.chop(1);
            this->title = line;
            continue;
        }
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            real_url.append(match.captured(1));
        }
    }
    if (real_url.isEmpty()) {
        qInfo() << "No valid url fetched!";
        exit(1);
    }

    setRes();

    genEDLUrl();

    auto sl = real_url[0].split('/', Qt::SkipEmptyParts);
    auto cid = sl.at(sl.length() - 2);
    //    qDebug() << cid;
    QNetworkRequest qnr("https://comment.bilibili.com/" + cid + ".xml");
    //    qDebug() << real_url;
    reply_dm = nam->get(qnr);
    connect(reply_dm, &QNetworkReply::finished, this, &BiliVideo::slotHttpDMXml);
}

void BiliVideo::playPage(int p)
{
    if (p < 1 || p > pages.length()) {
        return;
    }
    disconnect(mpv, &MpvControl::playFinished, this, &BiliVideo::autoNextPage);
    QString u;
    if (pages[p-1] == "") {
        u = base_url + "?p=" + QString::number(p);
    } else {
        u = "https://www.bilibili.com/bangumi/play/" + pages[p-1];
    }
    current_page = p;
    requestRealUrl(u);
}

void BiliVideo::autoNextPage()
{
    QTimer::singleShot(3000, [=]() {
        playPage(current_page + 1);
    });
}

void BiliVideo::goPrevPage()
{
    playPage(current_page - 1);
}

void BiliVideo::goNextPage()
{
    playPage(current_page + 1);
}
