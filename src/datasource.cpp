#include <algorithm>
#include "datasource.h"

using namespace DS;

DataSource::DataSource(RM::RoomModel *room_model, QObject *parent)
    : QObject(parent)
{
    this->room_model = room_model;
    this->base_model = room_model->base_model;

    connect(&sites, &Sites::roomDecoded, this, &DataSource::addRoom);
}

DataSource::~DataSource()
{
    saveSettings();
}

void DataSource::addRoom(QString url, QString title, QString owner, QString cover, int status, bool open)
{
//    qDebug() << url << title << owner << cover << status;
    auto c = urlToCode(url);
    if (open) {
        openQlphelper(url);
    }
    if (!rooms.contains(c)) {
        return;
    }
    rooms[c].url = url;
    rooms[c].title = title;
    rooms[c].owner = owner;
    rooms[c].cover = cover;
    rooms[c].status = status == 1 ? true : false;
    QString tag;
    if (rooms[c].like) {
        tag.append('S');
    }
    if (rooms[c].his) {
        tag.append('H');
    }
    RM::Room room(url, title, cover, owner, status, rooms[c].like ? 0 : 1);
    const int new_row = base_model->rowCount();
    rooms[c].row = new_row;
    base_model->insertRow(new_row);
    base_model->setData(base_model->index(new_row,0), QVariant::fromValue(room), Qt::DisplayRole);
    base_model->setData(base_model->index(new_row,1), QVariant::fromValue(tag));
    base_model->setData(base_model->index(new_row,2), QVariant::fromValue(status));
    base_model->setData(base_model->index(new_row,3), QVariant::fromValue(rooms[c].order_his));
    room_model->sort();
}

void DataSource::openQlphelper(QString url)
{
    auto c = urlToCode(url);
    if (!c.isEmpty()) {
        addHistory(c);
    }
    QProcess::startDetached("qlphelper", QStringList() << "-u" << url);
}

void DataSource::refresh()
{
    clear();
    for (const auto& c : rooms.keys()) {
        sites.checkUrl(c);
    }
}

void DataSource::openUrl(QString url)
{
    QStringList sl;
    sl = url.split('-', Qt::SkipEmptyParts);
    if (sl.size() >= 2) {
        sites.checkUrl(url, true);
    } else if (sl.size() == 1) {
        if (sl[0].left(2) == "av") {
            auto psl = sl[0].split(':', Qt::SkipEmptyParts);
            openQlphelper("https://www.bilibili.com/video/" + psl[0] + "?p=" + (psl.size() == 2 ? psl[1] : "1"));
        } else if (sl[0].left(2) == "ep") {
            openQlphelper("https://www.bilibili.com/bangumi/play/" + sl[0]);
        } else if (sl[0].left(2) == "BV") {
            auto psl = sl[0].split(':', Qt::SkipEmptyParts);
            openQlphelper("https://www.bilibili.com/video/" + psl[0] + "?p=" + (psl.size() == 2 ? psl[1] : "1"));
        } else if (sl[0].left(2) == "ss") {
            auto psl = sl[0].split(':', Qt::SkipEmptyParts);
            openQlphelper("https://www.bilibili.com/bangumi/play/" + psl[0] + "?p=" + (psl.size() == 2 ? psl[1] : "1"));
        } else if (sl[0].left(4) == "http") {
            if (!sl[0].contains("bilibili")) {
                return;
            }
            QUrl url(sl[0]);
            QString bi_code;
            bi_code = url.fileName();
            if (bi_code.left(2) != "av" && bi_code.left(2) != "ep" && bi_code.left(2) != "BV") {
                return;
            }
            openQlphelper(sl[0]);
        }
    }
}

void DataSource::toggleLike(int like, QString url)
{
    auto c = urlToCode(url);
    // 0 for like 1 for dislike
    if (!rooms.contains(c)) {
        return;
    }
    rooms[c].like = like == 0 ? true : false;
}

void DataSource::loadSettings()
{
    QSettings s("QLivePlayer", "QLivePlayer");

    for (auto& code : s.value("saved").toStringList()) {
        Room r;
        r.like = true;
        rooms[code] = r;
    }
    int i = 1;
    for (auto& code : s.value("history").toStringList()) {
        if (rooms.contains(code)) {
            rooms[code].his = true;
            rooms[code].order_his = i;
        } else {
            Room r;
            r.his = true;
            r.order_his = i;
            rooms[code] = r;
        }
        ++i;
    }
}

void DataSource::saveSettings()
{
    QSettings s("QLivePlayer", "QLivePlayer");
    QStringList saved;
    QStringList history;
    QMap<int, QString> tmp;
    for (const auto& c : rooms.keys()) {
        if (rooms[c].like) {
            saved.append(c);
        }
        if (rooms[c].his) {
            tmp.insert(rooms[c].order_his, c);
        }
    }
    for (const auto& r : tmp) {
        history.append(r);
    }
    s.setValue("saved", saved);
    s.setValue("history", QStringList(history.mid(0, 24)));
}

inline
QString DataSource::urlToCode(QString url)
{
    if (url.contains("douyu.com/")) {
        QString rid = url.split('/', Qt::SkipEmptyParts).last();
        return "do-" + rid;
    } else if (url.contains("live.bilibili.com/")) {
        QString rid = url.split('/', Qt::SkipEmptyParts).last();
        return "bi-" + rid;
    } else if (url.contains("huya.com/")) {
        QString rid = url.split('/', Qt::SkipEmptyParts).last();
        return "hu-" + rid;
    } else if (url.contains("youtube.com/channel/")) {
        QString rid = url.split('/', Qt::SkipEmptyParts).last();
        return "yt-" + rid;
    } else if (url.contains("youtube.com/c/")) {
        QString rid = url.split('/', Qt::SkipEmptyParts).last();
        return "ytv-" + rid;
    } else if (url.contains("twitch.tv/")) {
        QString rid = url.split('/', Qt::SkipEmptyParts).last();
        return "tw-" + rid;
    }
    return "";
}

void DataSource::clear()
{
    base_model->clear();
    base_model->insertColumn(0);
    base_model->insertColumn(1);
    base_model->insertColumn(2);
    base_model->insertColumn(3);
    for (const auto& c : rooms.keys()) {
        rooms[c].row = -1;
    }
}

void DataSource::addHistory(QString code)
{
    if (!rooms.contains(code)) {
        Room r;
        r.his = true;
        r.order_his = 1;
        for (auto& c : rooms.keys()) {
            if (rooms[c].his) {
                ++rooms[c].order_his;
                base_model->setData(base_model->index(rooms[c].row, 3), QVariant::fromValue(rooms[c].order_his));
            }
        }
        rooms[code] = r;
    } else {
        for (auto& c : rooms.keys()) {
            if (rooms[c].his && rooms[c].order_his < rooms[code].order_his) {
                ++rooms[c].order_his;
                base_model->setData(base_model->index(rooms[c].row, 3), QVariant::fromValue(rooms[c].order_his));
            }
        }
        rooms[code].his = true;
        rooms[code].order_his = 1;
        QString tag;
        if (rooms[code].like) {
            tag.append('S');
        }
        tag.append('H');
        base_model->setData(base_model->index(rooms[code].row, 1), QVariant::fromValue(tag));
        base_model->setData(base_model->index(rooms[code].row, 3), QVariant::fromValue(rooms[code].order_his));
    }
    room_model->sort();
}
