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

DanmakuLauncher::DanmakuLauncher(QString room_url, QString danmaku_socket)
{
    fk = new FudujiKiller();
    this->danmaku_socket_path = danmaku_socket;
    this->room_url = room_url;

    launch_timer = new QTimer(this);
    launch_timer->setInterval(200);

    socket_server = new QLocalServer(this);
    connect(socket_server, &QLocalServer::newConnection, this, &DanmakuLauncher::setSocket);
    socket_server->listen(danmaku_socket);

    for (int i = 0; i < 30; i++) {
        danmaku_channel[i].length = 1;
        danmaku_channel[i].begin_pts = -10000;
    }

    dmcPyProcess = new QProcess();
    connect(dmcPyProcess, &QProcess::readyRead, this, &DanmakuLauncher::loadDanmaku);
    connect(launch_timer, &QTimer::timeout, this, &DanmakuLauncher::launchDanmaku);
}

DanmakuLauncher::~DanmakuLauncher()
{
    launch_timer->stop();
    delete fk;
    dmcPyProcess->terminate();
    dmcPyProcess->waitForFinished(3000);
    delete dmcPyProcess;
}

void DanmakuLauncher::startDmcPy()
{
    dmcPyProcess->terminate();
    dmcPyProcess->waitForFinished();
    QStringList dmcPy;
    dmcPy.append(QStandardPaths::locate(QStandardPaths::DataLocation, "dmc.pyz"));
    dmcPy.append(room_url);
    dmcPyProcess->start("python3", dmcPy);
    qDebug() << "Danmaku process started!";
}

void DanmakuLauncher::setScale(int w, int h)
{
    scale = 16.0 * h / w / 9.0;
    qDebug() << w << h << scale;
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
                ass_event = QString("%4,0,Default,%5,0,0,0,,{\\move(1920,%1,%2,%1)}%3").arg(QString().number(avail_dc*(font_size))).arg(QString().number(0-display_length)).arg((*iter).second).arg(QString().number(read_order)).arg((*iter).first);
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
    socket->write(bin_out);
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
    socket->write(bin_out);
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

void DanmakuLauncher::start()
{

}

void DanmakuLauncher::restart()
{
    launch_timer->stop();
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    for (int i = 0; i < 30; i++) {
        danmaku_channel[i].length = 1;
        danmaku_channel[i].begin_pts = -10000;
    }
}

void DanmakuLauncher::stop()
{
    launch_timer->stop();
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
}

void DanmakuLauncher::onStreamStart()
{

}

void DanmakuLauncher::setSocket()
{
    if (socket != nullptr) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }
    socket = socket_server->nextPendingConnection();
    startDmcPy();
    timer.restart();
    launch_timer->start(200);
    socket->write(QByteArray(reinterpret_cast<const char*>(mkv_header), mkv_header_len));
}
