#include "room_model.h"
#include "sites.h"

RoomModel::RoomModel(QObject *parent) : QObject(parent)
{
    m_saved_model = new QStandardItemModel(this);
    m_history_model = new QStandardItemModel(this);
    connect(&sites, &Sites::roomDecoded, this, &RoomModel::addRoomToModel);
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

QString RoomModel::urlToCode(QString url)
{
    if (url.contains("douyu")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "do-" + rid;
    } else if (url.contains("bilibili")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "bi-" + rid;
    } else if (url.contains("huya")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        return "hu-" + rid;
    }
    return "";
}

void RoomModel::openRoom(QString url)
{
    if (url.contains("douyu")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        history.prepend("do-" + rid);
    } else if (url.contains("bilibili")) {
        if (url.contains("/video") || url.contains("/bangumi")) {

        } else {
            QString rid = url.split('/', QString::SkipEmptyParts).last();
            history.prepend("bi-" + rid);
        }
    } else if (url.contains("huya")) {
        QString rid = url.split('/', QString::SkipEmptyParts).last();
        history.prepend("hu-" + rid);
    }
    history.removeDuplicates();
    refreshHistory();
    QProcess::startDetached("qlphelper", QStringList() << "-u" << url);
}

void RoomModel::refresh()
{
    QStringList sl;
    for (auto& s : saved) {
        sl = s.split('-');
        if (sl[0] == "do") {
            sites.checkUrl("https://www.douyu.com/betard/" + sl[1], 0);
        } else if (sl[0] == "bi") {
            sites.checkUrl("https://api.live.bilibili.com/xlive/web-room/v1/index/getInfoByRoom?room_id=" + sl[1], 0);
        } else if (sl[0] == "hu") {
            sites.checkUrl("https://m.huya.com/" + sl[1], 0);
        }
    }
    int i = 1;
    for (auto& s : history) {
        sl = s.split('-');
        if (sl[0] == "do") {
            sites.checkUrl("https://www.douyu.com/betard/" + sl[1], 1, i);
        } else if (sl[0] == "bi") {
            sites.checkUrl("https://api.live.bilibili.com/xlive/web-room/v1/index/getInfoByRoom?room_id=" + sl[1], 1, i);
        } else if (sl[0] == "hu") {
            sites.checkUrl("https://m.huya.com/" + sl[1], 1, i);
        }
        if (i >= 12) {
            break;
        } else {
            ++i;
        }
    }
    m_saved_model->clear();
    m_saved_model->insertColumn(0);
    m_saved_model->insertColumn(1);
    m_history_model->clear();
    m_history_model->insertColumn(0);
    m_history_model->insertColumn(1);
}

void RoomModel::refreshHistory()
{
    int i = 1;
    QStringList sl;
    for (auto& s : history) {
        sl = s.split('-');
        if (sl[0] == "do") {
            sites.checkUrl("https://www.douyu.com/betard/" + sl[1], 1, i);
        } else if (sl[0] == "bi") {
            sites.checkUrl("https://api.live.bilibili.com/xlive/web-room/v1/index/getInfoByRoom?room_id=" + sl[1], 1, i);
        } else if (sl[0] == "hu") {
            sites.checkUrl("https://m.huya.com/" + sl[1], 1, i);
        }
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
    if (sl.size() == 2) {
        if (sl[0] == "do") {
            openRoom("https://www.douyu.com/" + sl[1]);
        } else if (sl[0] == "bi") {
            openRoom("https://live.bilibili.com/" + sl[1]);
        } else if (sl[0] == "hu") {
            openRoom("https://www.huya.com/" + sl[1]);
        }
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
    // 0 for like 1 for dislike
    if (like == 0) {
        if (url.contains("douyu")) {
            QString rid = url.split('/', QString::SkipEmptyParts).last();
            saved.prepend("do-" + rid);
        } else if (url.contains("bilibili")) {
            QString rid = url.split('/', QString::SkipEmptyParts).last();
            saved.prepend("bi-" + rid);
        } else if (url.contains("huya")) {
            QString rid = url.split('/', QString::SkipEmptyParts).last();
            saved.prepend("hu-" + rid);
        }
        saved.removeDuplicates();
    } else {
        if (url.contains("douyu")) {
            QString rid = url.split('/', QString::SkipEmptyParts).last();
            int i = saved.indexOf("do-" + rid);
            if (i != -1) {
                saved.removeAt(i);
            }
        } else if (url.contains("bilibili")) {
            QString rid = url.split('/', QString::SkipEmptyParts).last();
            int i = saved.indexOf("bi-" + rid);
            if (i != -1) {
                saved.removeAt(i);
            }
        } else if (url.contains("huya")) {
            QString rid = url.split('/', QString::SkipEmptyParts).last();
            int i = saved.indexOf("hu-" + rid);
            if (i != -1) {
                saved.removeAt(i);
            }
        }
    }
}
