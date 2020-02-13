#include <QtCore>
#include <QProcess>
#include <cstdlib>
#include <csignal>
#include "danmakulauncher.h"
#include "mkv_header.h"

#define __PICK(n, i)   (((n)>>(8*i))&0xFF)

void stopSubProcesses(qint64 parentProcessId) {
    qDebug() << "stop subprocess for parent id:" << parentProcessId;
    QProcess get_childs;
    QStringList get_childs_cmd;
    get_childs_cmd << "--ppid" << QString::number(parentProcessId) << "-o" << "pid" << "--no-heading";
    get_childs.start("ps", get_childs_cmd);
    get_childs.waitForFinished();
    QString childIds(get_childs.readAllStandardOutput());
    childIds.replace('\n', ' ');

    QProcess::execute("kill " + childIds);
}

DanmakuLauncher::DanmakuLauncher(QStringList args)
{
    fk = new FudujiKiller();
    fifo = QString("/tmp/qlp-%1").arg(QUuid::createUuid().toString());
    url = args.at(0);
    record_file = args.at(1);
    if (args.at(2) == "true") {
        no_window = true;
    }
    if (args.at(3) == "true") {
        is_debug = true;
    }
    stream_url = getStreamUrl(url);
    if (stream_url == "null") {
        exit(1);
        return;
    }

    launch_timer = new QTimer(this);
    launch_timer->setInterval(200);
    launch_timer->start();

    QProcess::execute(QString("mkfifo %1").arg(fifo));
    fifo_file = new QFile(fifo);
    initPlayer();

    if (!fifo_file->open(QIODevice::WriteOnly)) {
        qCritical() << "Cannot open fifo!";
        exit(1);
    }
    fifo_file->write(reinterpret_cast<const char*>(mkv_header), mkv_header_len);
    fifo_file->flush();

    for (int i = 0; i < 30; i++) {
        danmaku_channel[i].length = 1;
        danmaku_channel[i].begin_pts = -10000;
    }

    initDmcPy();
    live_checker = new QProcess(this);
    connect(launch_timer, &QTimer::timeout, this, &DanmakuLauncher::launchDanmaku);
//    connect(live_checker, &QProcess::readyRead, this, &DanmakuLauncher::getLiveStatus);

}

DanmakuLauncher::~DanmakuLauncher()
{
    qCritical() << "Bye!";
    delete fk;
    launch_timer->stop();
    launch_timer->deleteLater();
    fifo_file->close();
    fifo_file->remove();
    fifo_file->deleteLater();
    live_checker->disconnect();
    player->disconnect();
    dmcPyProcess->disconnect();
    if (player->processId() != 0) {
        stopSubProcesses(player->processId());
    }
    live_checker->terminate();
    live_checker->waitForFinished(3000);
    live_checker->deleteLater();
    dmcPyProcess->terminate();
    dmcPyProcess->waitForFinished(3000);
    dmcPyProcess->deleteLater();
    player->terminate();
    player->waitForFinished(3000);
    player->deleteLater();
}

void DanmakuLauncher::initDmcPy()
{
    timer.start();
    QStringList dmcPy;
    dmcPy.append(QStandardPaths::locate(QStandardPaths::DataLocation, "dmc.pyz"));
    dmcPy.append(url);
    dmcPyProcess = new QProcess(this);
    connect(dmcPyProcess, &QProcess::readyRead, this, &DanmakuLauncher::loadDanmaku);
    dmcPyProcess->start("python3", dmcPy);
    qDebug() << "Danmaku process initialized!";
}

void DanmakuLauncher::initPlayer()
{
    QStringList player_args;

    player_args.append("-c");
    player_args.append(getPlayerCMD(url));
    player = new QProcess(this);
    player->setReadChannel(QProcess::StandardError);
    connect(player, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus exitStatus){
        Q_UNUSED(exitStatus);
        QCoreApplication::exit(exitCode);
    });
    setScale();
//    connect(player, &QProcess::readyReadStandardError, this, &DanmakuLauncher::printPlayerProcess);
    player->start("sh", player_args);
    qDebug() << "Player process initialized!";
}

void DanmakuLauncher::setScale()
{
    QString ua("User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.88 Safari/537.36");
    auto p = new QProcess(this);
    p->start("ffmpeg", QStringList() << "-user_agent" << ua << "-i" << stream_url);
    p->setReadChannel(QProcess::StandardError);
    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus exitStatus){
        Q_UNUSED(exitStatus);
        Q_UNUSED(exitCode);
        QRegularExpression re(" ([0-9]+)x([0-9]+),");
        QRegularExpressionMatch match = re.match(p->readAllStandardError());
        if (match.hasMatch()) {
             scale = 16.0 * match.captured(2).toDouble() / match.captured(1).toDouble() / 9.0;
             qDebug() << match.captured(1) << match.captured(2) << scale;
        }
    });
}

QString DanmakuLauncher::getPlayerCMD(QString url)
{
    QString ret;
    QString mpv_extra_args;
    QString record_args;
    QString ua("User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.88 Safari/537.36");
    record_args = record_file.compare("null") != 0 ? QString("tee '%1' | ").arg(record_file) : " ";
    mpv_extra_args = (no_window ?
                          " --no-video --mute=yes --player-operation-mode=cplayer --term-status-msg='${?paused-for-cache==yes:buffering;}${?paused-for-cache==no:playing;}'" :
                          " --player-operation-mode=pseudo-gui ");
    if (url.contains("huya.com")) {
        ret = QString("ffmpeg -loglevel %8 -user_agent '%6' -i '%1' -c copy -f flv - %7| "
                      "ffmpeg -i '%2' -i - -loglevel quiet -map 1:v -map 1:a -map 0:0 -c copy -f matroska -metadata title='%4' - | "
                      "%5 mpv %3 --vf='lavfi=\"fps=fps=60:round=down\"' -")
                .arg(stream_url).arg(fifo).arg(mpv_extra_args).arg(title).arg(record_args).arg(ua)
                .arg(is_debug ? "2>/tmp/fflog" : " ").arg(is_debug ? "debug" : "quiet");
    } else {
        ret = QString("ffmpeg -loglevel %8 -i '%2' -user_agent '%6' -i '%1' -map 1:v -map 1:a -map 0:0 -c copy -f matroska -metadata title='%4' - %7| "
                      "%5 mpv %3 --vf='lavfi=\"fps=fps=60:round=down\"' -")
                .arg(stream_url).arg(fifo).arg(mpv_extra_args).arg(title).arg(record_args).arg(ua)
                .arg(is_debug ? "2>/tmp/fflog" : " ").arg(is_debug ? "debug" : "quiet");
    }
    return ret;
}

int DanmakuLauncher::getDankamuDisplayLength(QString dm, int fontsize)
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
    return ((fontsize * 0.75 * dm.size()) - (fontsize * 0.25 * ascii_num)) * scale;
}


void DanmakuLauncher::loadDanmaku()
{
    while(dmcPyProcess->canReadLine()) {
        danmaku_queue.enqueue(dmcPyProcess->readLine());
    }
}

void DanmakuLauncher::launchDanmaku()
{
    fk->addTime();
    fk->ebb();
    if (check_counter % 50 == 49) {
        getLiveStatus();
        check_counter = 0;
    } else {
        ++check_counter;
    }
    launchVoidDanmaku();
    if (danmaku_queue.isEmpty()) {
        return;
    }
    int display_length = 0;
    int avail_dc = -1;
    QString dm;
    QString speaker;
    QString ass_event;
    QByteArray bin_out;
    quint64 buf;
    while (!danmaku_queue.isEmpty()) {
        dm = danmaku_queue.dequeue();
        dm.chop(1);
        qInfo().noquote() << dm;
        dm.remove(0, 1);
        speaker = dm.section(QChar(']'), 0, 0);
        dm = dm.section(QChar(']'), 1, -1);
        dm.remove(0, 1);
        if (!fk->checkDanmaku(dm)) {
            continue;
        }
        dm.replace(QRegularExpression("[\\x{1F300}-\\x{1F5FF}|\\x{1F1E6}-\\x{1F1FF}|\\x{2700}-\\x{27BF}|\\x{1F900}-\\x{1F9FF}|\\x{1F600}-\\x{1F64F}|\\x{1F680}-\\x{1F6FF}|\\x{2600}-\\x{26FF}]"), "[em]"); // remove emoji
        display_length = getDankamuDisplayLength(dm, font_size);
        avail_dc = getAvailDanmakuChannel(display_length);
        if (avail_dc >= 0) {
            QByteArray tmp;
            ass_event = QString("%4,0,Default,%5,0,0,0,,{\\move(1920,%1,%2,%1)}%3").arg(QString().number(avail_dc*(font_size))).arg(QString().number(0-display_length)).arg(dm).arg(QString().number(read_order)).arg(speaker);
            ++read_order;
            tmp = ass_event.toLocal8Bit();
            tmp.prepend((char)0x00);
            tmp.prepend((char)0x00);
            tmp.prepend((char)0x00);
            tmp.prepend((char)0x81);
            buf = tmp.size() | 0x10000000;
            tmp.prepend(__PICK(buf, 0));
            tmp.prepend(__PICK(buf, 1));
            tmp.prepend(__PICK(buf, 2));
            tmp.prepend(__PICK(buf, 3));
            tmp.prepend(0xa1);
            buf = 0x849b;
            tmp.append((const char *)&buf, 2);
            buf = this->speed;
            tmp.append(__PICK(buf, 3));
            tmp.append(__PICK(buf, 2));
            tmp.append(__PICK(buf, 1));
            tmp.append(__PICK(buf, 0));
            buf = tmp.size() | 0x10000000;
            tmp.prepend(__PICK(buf, 0));
            tmp.prepend(__PICK(buf, 1));
            tmp.prepend(__PICK(buf, 2));
            tmp.prepend(__PICK(buf, 3));
            tmp.prepend(0xa0);
            bin_out.append(tmp);
        } else {
            droped_danmaku_list.append(QPair<QString, QString>(speaker, dm));
            continue;
        }
    }
    if (!droped_danmaku_list.isEmpty()) {
//        qDebug() << "droped list size: " << droped_danmaku_list.size();
        auto iter = droped_danmaku_list.begin();
        while (iter < droped_danmaku_list.end()) {
            display_length = getDankamuDisplayLength((*iter).second, font_size);
            avail_dc = getAvailDanmakuChannel(display_length);
            if (avail_dc >= 0) {
                QByteArray tmp;
                ass_event = QString("%4,0,Default,%5,0,0,0,,{\\1c&02D3FF&\\move(1920,%1,%2,%1)}%3").arg(QString().number(avail_dc*(font_size))).arg(QString().number(0-display_length)).arg((*iter).second).arg(QString().number(read_order)).arg((*iter).first);
                ++read_order;
                tmp = ass_event.toLocal8Bit();
                tmp.prepend((char)0x00);
                tmp.prepend((char)0x00);
                tmp.prepend((char)0x00);
                tmp.prepend((char)0x81);
                buf = tmp.size() | 0x10000000;
                tmp.prepend(__PICK(buf, 0));
                tmp.prepend(__PICK(buf, 1));
                tmp.prepend(__PICK(buf, 2));
                tmp.prepend(__PICK(buf, 3));
                tmp.prepend(0xa1);
                buf = 0x849b;
                tmp.append((const char *)&buf, 2);
                buf = this->speed;
                tmp.append(__PICK(buf, 3));
                tmp.append(__PICK(buf, 2));
                tmp.append(__PICK(buf, 1));
                tmp.append(__PICK(buf, 0));
                buf = tmp.size() | 0x10000000;
                tmp.prepend(__PICK(buf, 0));
                tmp.prepend(__PICK(buf, 1));
                tmp.prepend(__PICK(buf, 2));
                tmp.prepend(__PICK(buf, 3));
                tmp.prepend(0xa0);
                bin_out.append(tmp);
                iter = droped_danmaku_list.erase(iter);
            } else {
                ++iter;
            }
        }
        if (droped_danmaku_list.size() > 500) {
            droped_danmaku_list.clear();
        }
    }
    buf = (pts + timer.elapsed());
    bin_out.prepend(__PICK(buf, 0));
    bin_out.prepend(__PICK(buf, 1));
    bin_out.prepend(__PICK(buf, 2));
    bin_out.prepend(__PICK(buf, 3));
    bin_out.prepend(__PICK(buf, 4));
    bin_out.prepend(__PICK(buf, 5));
    bin_out.prepend(__PICK(buf, 6));
    bin_out.prepend(__PICK(buf, 7));
    bin_out.prepend(0x88);
    bin_out.prepend(0xe7);
    buf = bin_out.size() | 0x0100000000000000;
    bin_out.prepend(__PICK(buf, 0));
    bin_out.prepend(__PICK(buf, 1));
    bin_out.prepend(__PICK(buf, 2));
    bin_out.prepend(__PICK(buf, 3));
    bin_out.prepend(__PICK(buf, 4));
    bin_out.prepend(__PICK(buf, 5));
    bin_out.prepend(__PICK(buf, 6));
    bin_out.prepend(__PICK(buf, 7));
    buf = 0x1f43b675;
    bin_out.prepend(__PICK(buf, 0));
    bin_out.prepend(__PICK(buf, 1));
    bin_out.prepend(__PICK(buf, 2));
    bin_out.prepend(__PICK(buf, 3));
    fifo_file->write(bin_out);
    fifo_file->flush();
}

void DanmakuLauncher::launchVoidDanmaku()
{
    QString ass_event;
    QByteArray bin_out;
    quint64 buf;
    QByteArray tmp;
    ass_event = QString("%1,0,Default,QLivePlayer-Empty,20,20,2,,").arg(QString().number(read_order));
    ++read_order;
    tmp = ass_event.toLocal8Bit();
    tmp.prepend((char)0x00);
    tmp.prepend((char)0x00);
    tmp.prepend((char)0x00);
    tmp.prepend((char)0x81);
    buf = tmp.size() | 0x10000000;
    tmp.prepend(__PICK(buf, 0));
    tmp.prepend(__PICK(buf, 1));
    tmp.prepend(__PICK(buf, 2));
    tmp.prepend(__PICK(buf, 3));
    tmp.prepend(0xa1);
    tmp.append(0x9b);
    tmp.append(0x84);
    buf = 1;
    tmp.append(__PICK(buf, 3));
    tmp.append(__PICK(buf, 2));
    tmp.append(__PICK(buf, 1));
    tmp.append(__PICK(buf, 0));
    buf = tmp.size() | 0x10000000;
    tmp.prepend(__PICK(buf, 0));
    tmp.prepend(__PICK(buf, 1));
    tmp.prepend(__PICK(buf, 2));
    tmp.prepend(__PICK(buf, 3));
    tmp.prepend(0xa0);
    bin_out.append(tmp);
    buf = (pts + timer.elapsed());
    bin_out.prepend(__PICK(buf, 0));
    bin_out.prepend(__PICK(buf, 1));
    bin_out.prepend(__PICK(buf, 2));
    bin_out.prepend(__PICK(buf, 3));
    bin_out.prepend(__PICK(buf, 4));
    bin_out.prepend(__PICK(buf, 5));
    bin_out.prepend(__PICK(buf, 6));
    bin_out.prepend(__PICK(buf, 7));
    bin_out.prepend(0x88);
    bin_out.prepend(0xe7);
    buf = bin_out.size() | 0x0100000000000000;
    bin_out.prepend(__PICK(buf, 0));
    bin_out.prepend(__PICK(buf, 1));
    bin_out.prepend(__PICK(buf, 2));
    bin_out.prepend(__PICK(buf, 3));
    bin_out.prepend(__PICK(buf, 4));
    bin_out.prepend(__PICK(buf, 5));
    bin_out.prepend(__PICK(buf, 6));
    bin_out.prepend(__PICK(buf, 7));
    buf = 0x1f43b675;
    bin_out.prepend(__PICK(buf, 0));
    bin_out.prepend(__PICK(buf, 1));
    bin_out.prepend(__PICK(buf, 2));
    bin_out.prepend(__PICK(buf, 3));
    fifo_file->write(bin_out);
    fifo_file->flush();
}

int DanmakuLauncher::getAvailDanmakuChannel(int len)
{
    double s = (1920.0 + len) / this->speed;
    qint64 c_pts = timer.elapsed() + pts;
    for(int i = 0; i < channel_num; i++)
    {
        if (((this->speed - c_pts + danmaku_channel[i].begin_pts) * s) > 1920) {
            continue;
        } else {
            if (((danmaku_channel[i].length + 1920.0) * (c_pts - danmaku_channel[i].begin_pts) / this->speed) < danmaku_channel[i].length ) {
                continue;
            } else {
                danmaku_channel[i].length = len;
                danmaku_channel[i].begin_pts = c_pts;
                return i;
            }
        }
    }
    return -4;
}

void DanmakuLauncher::printPlayerProcess()
{
    while (!player->atEnd()) {
        QString err = player->readAll();
        qDebug() << err;
        if (err.contains("The specified session has been invalidated for some reason")) {
            qCritical() << "Stream invalid!";
            QCoreApplication::exit(1);
        }
    }
}

void DanmakuLauncher::getLiveStatus()
{
    if (!no_window) {
        return;
    }
    QString err;
    if (!player->atEnd()) {
        err = player->readAll();
        qDebug() << err;
    }
    if (err.isEmpty()) {
        if (on_buffering == true) {
            qCritical() << "Network error! Closing...";
            QCoreApplication::exit(1);
        }
        return;
    }

    QStringList sl = err.split(';', QString::SkipEmptyParts);

    for (auto s : sl) {
        qDebug() << s;
        if (s.contains("buffering")) {
            on_buffering = true;
        } else if (s.contains("playing")) {
            on_buffering = false;
        }
    }
}

void DanmakuLauncher::launchLiveChecker()
{
//    qDebug() << live_checker->state();
    if (live_checker->state() == QProcess::NotRunning) {
        if (!live_checker->canReadLine()) {
            live_checker->start("ykdl", QStringList() << "-i" << url);
        }
    }
}

QString DanmakuLauncher::getStreamUrl(QString url)
{
    QProcess p;
    p.start("ykdl", QStringList() << "-i" << url);
    p.waitForStarted(5000);
    p.waitForFinished(5000);
    QRegularExpression re("^(http.+)$");
    QRegularExpression re_title("^title: +([^\n]+)$");
    while(!p.atEnd()) {
        QString line(p.readLine());
        QRegularExpressionMatch match = re_title.match(line);
        if (match.hasMatch()) {
             this->title = match.captured(1);
        }
        match = re.match(line);
        if (match.hasMatch()) {
             return match.captured(1);
        }
    }
    qCritical() << "Stream url not found!";
    return "null";
}


