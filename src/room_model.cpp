#include "room_model.h"
#include "sites.h"

RoomModel::RoomModel(QObject *parent) : QObject(parent)
{
    m_saved_model = new QStandardItemModel(this);
    m_history_model = new QStandardItemModel(this);
    connect(&sites, &Sites::roomDecoded, this, &RoomModel::addRoomToModel);
    connect(&sites, &Sites::urlVerified, this, &RoomModel::openRoom);
}

RoomModel::~RoomModel()
{
    save();
}

QStandardItemModel *RoomModel::saved_model() const
{
    return m_saved_model;
}

QStandardItemModel *RoomModel::history_model() const
{
    return m_history_model;
}

void RoomModel::load()
{
    settings = new QSettings("QLivePlayer", "QLivePlayer", this);

    saved = settings->value("saved").toStringList();
    history = settings->value("history").toStringList();

}

void RoomModel::addRoomToModel(int cata, QString url, QString title, QString owner, QString cover, int status, int num)
{
    // cata: 0 for saved 1 for history
//    qDebug() << cata << url << title << owner << cover << status;
    if (cata == 0) {
        Room room(url, title, cover, owner, status, cata);
        const int new_row = m_saved_model->rowCount();
        m_saved_model->insertRow(new_row);
        m_saved_model->setData(m_saved_model->index(new_row,0), QVariant::fromValue(room));
        m_saved_model->setData(m_saved_model->index(new_row,1), QVariant::fromValue(status));
        m_saved_model->sort(1, Qt::DescendingOrder);
    } else if (cata == 1) {
        Room room(url, title, cover, owner, status, saved.contains(urlToCode(url)) ? 0 : 1);
        m_history_model->insertRow(0);
        m_history_model->setData(m_history_model->index(0,0), QVariant::fromValue(room));
        m_history_model->setData(m_history_model->index(0,1), QVariant::fromValue(num));
        m_history_model->sort(1, Qt::AscendingOrder);
    }
}

void RoomModel::save()
{
    settings->setValue("saved", saved);
    settings->setValue("history", QStringList(history.mid(0, 12)));
}

inline
QString RoomModel::urlToCode(QString url)
{
    if (url.contains("douyu.com/")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "do-" + rid;
    } else if (url.contains("live.bilibili.com/")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "bi-" + rid;
    } else if (url.contains("huya.com/")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "hu-" + rid;
    } else if (url.contains("youtube.com/channel/")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "yt-" + rid;
    } else if (url.contains("youtube.com/c/")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "ytv-" + rid;
    } else if (url.contains("twitch.tv/")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "tw-" + rid;
    }
    return "";
}

void RoomModel::addUrlToHistory(QString url)
{
    QProcess::startDetached("qlphelper", QStringList() << "-u" << url);
    auto a = urlToCode(url);
    if (!a.isEmpty()) {
        history.prepend(a);
        history.removeDuplicates();
        refresh(true);
    }
}

void RoomModel::openRoom(QString url)
{
    auto a = urlToCode(url);
    if (!a.isEmpty()) {
        history.prepend(a);
        history.removeDuplicates();
        refresh(true);
    }
    QProcess::startDetached("qlphelper", QStringList() << "-u" << url);
}

void RoomModel::openUnverifiedRoom(QString url)
{
    sites.checkUnverifiedUrl(url);
}

void RoomModel::refresh(bool history_only)
{
    QStringList sl;
    if (history_only == false) {
        for (auto& s : saved) {
            sites.checkUrl(s, 0);
        }
        m_saved_model->clear();
        m_saved_model->insertColumn(0);
        m_saved_model->insertColumn(1);
    }
    int i = 1;
    for (auto& s : history) {
        sites.checkUrl(s, 1, i);
        if (i >= 12) {
            break;
        } else {
            ++i;
        }
    }
    m_history_model->clear();
    m_history_model->insertColumn(0);
    m_history_model->insertColumn(1);
}

void RoomModel::openUrl(QString url)
{
    QStringList sl;
    sl = url.split('-', QString::SkipEmptyParts);
    if (sl.size() >= 2) {
        openUnverifiedRoom(url);
    } else if (sl.size() == 1) {
        if (sl[0].left(2) == "av") {
            auto psl = sl[0].split(':', QString::SkipEmptyParts);
            openRoom("https://www.bilibili.com/video/" + psl[0] + "?p=" + (psl.size() == 2 ? psl[1] : "1"));
        } else if (sl[0].left(2) == "ep") {
            openRoom("https://www.bilibili.com/bangumi/play/" + sl[0]);
        } else if (sl[0].left(2) == "BV") {
            auto psl = sl[0].split(':', QString::SkipEmptyParts);
            openRoom("https://www.bilibili.com/video/" + psl[0] + "?p=" + (psl.size() == 2 ? psl[1] : "1"));
        } else if (sl[0].left(2) == "ss") {
            auto psl = sl[0].split(':', QString::SkipEmptyParts);
            openRoom("https://www.bilibili.com/bangumi/play/" + psl[0] + "?p=" + (psl.size() == 2 ? psl[1] : "1"));
        } else if (sl[0].left(4) == "http") {
            if (!sl[0].contains("bilibili")) {
                return;
            }
            QUrl url(sl[0]);
            QString bi_code, part;
            bi_code = url.fileName();
            if (bi_code.left(2) != "av" && bi_code.left(2) != "ep" && bi_code.left(2) != "BV") {
                return;
            }
            openRoom(sl[0]);
        }
    }
}

void RoomModel::toggleLike(int like, QString url)
{
    auto a = urlToCode(url);
    // 0 for like 1 for dislike
    if (like == 0) {
        saved.prepend(a);
        saved.removeDuplicates();
    } else {
        int i = saved.indexOf(a);
        if (i != -1) {
            saved.removeAt(i);
        }
    }
}
