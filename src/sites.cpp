#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>

#include "sites.h"

#define qsl(s) QStringLiteral(s)

Sites::Sites(QObject* parent)
  : QObject(parent)
{
    nam = new QNetworkAccessManager(this);
    nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(nam, &QNetworkAccessManager::finished, this, &Sites::httpFinished);
}

void
Sites::checkUrl(QString url, bool open)
{
    auto sl = url.split('-');
    if (sl[0] == "do") {
        nam->get(genRequest("https://www.douyu.com/betard/" + sl[1], false, false, open));
    } else if (sl[0] == "bi") {
        nam->get(genRequest("https://api.live.bilibili.com/xlive/web-room/v1/index/getInfoByRoom?room_id=" + sl[1], false, false, open));
    } else if (sl[0] == "hu") {
        nam->get(genRequest("https://m.huya.com/" + sl[1], true, false, open));
    } else if (sl[0] == "yt") {
        nam->get(genRequest("https://www.youtube.com/channel/" + url.mid(3) + "/videos", false, true, open));
    } else if (sl[0] == "ytv") {
        nam->get(genRequest("https://www.youtube.com/embed/" + url.mid(4), false, true, open));
    } else if (sl[0] == "tw") {
        nam->get(genRequest("https://m.twitch.tv/" + url.mid(3) + "/profile", true, true, open));
    }
}

void
Sites::httpFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error: " << reply->error();
        reply->deleteLater();
        return;
    }
    bool open = reply->request().rawHeader("qlp-open") == "1" ? true : false;
    if (reply->request().url().toString().contains("douyu")) {
        QStringList sl = decodeDouyu(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://www.douyu.com/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        emit roomDecoded(url, sl[1], sl[2], sl[3], sl[4] == "1" ? 1 : 0, open);
    } else if (reply->request().url().toString().contains("bilibili")) {
        QStringList sl = decodeBilibili(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://live.bilibili.com/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        emit roomDecoded(url, sl[1], sl[2], sl[3], sl[4] == "1" ? 1 : 0, open);
    } else if (reply->request().url().toString().contains("huya")) {
        QStringList sl = decodeHuya(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://www.huya.com/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        emit roomDecoded(url, sl[1], sl[2], sl[3], sl[4] == "2" ? 1 : 0, open);
    } else if (reply->request().url().toString().contains("youtube.com/channel")) {
        QStringList sl = decodeYoutube(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://www.youtube.com/channel/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        emit roomDecoded(url, sl[1], sl[2], sl[3], sl[4] == "true" ? 1 : 0, open);
    } else if (reply->request().url().toString().contains("youtube.com/embed")) {
        QRegularExpression re_cid("\\\\\"channelId\\\\\":\\\\\"([^\"]+)\\\\\"");
        auto m = re_cid.match(reply->readAll());
        if (m.hasMatch()) {
            nam->get(genRequest("https://www.youtube.com/channel/" + m.captured(1) + "/videos", false, true, open));
        }
    } else if (reply->request().url().toString().contains("twitch.tv")) {
        QStringList sl = decodeTwitch(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://www.twitch.tv/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        emit roomDecoded(url, sl[1], sl[2], sl[3], sl[4] == "true" ? 1 : 0, open);
    }

    reply->deleteLater();
    return;
}

QStringList
Sites::decodeDouyu(const QByteArray& s)
{
    QStringList ret;
    QJsonDocument jd(QJsonDocument::fromJson(s));
    QJsonObject tmp;
    tmp = jd.object();
    if (!tmp.isEmpty()) {
        tmp = tmp.value("room").toObject();
        if (!tmp.isEmpty()) {
            if (tmp.value("room_id").toInt() != 0) {
                ret.append(QString::number(tmp.value("room_id").toInt()));
            } else {
                ret.append("");
            }
            ret.append(tmp.value("room_name").toString(""));
            ret.append(tmp.value("nickname").toString(""));
            ret.append(tmp.value("room_pic").toString(""));
            (tmp.value("show_status").toInt(2) == 1 && tmp.value("videoLoop").toInt(1) == 0) ? ret.append("1") : ret.append("");
        }
    }
    if (ret.length() != 5) {
        return QStringList() << ""
                             << ""
                             << ""
                             << ""
                             << "";
    } else {
        return ret;
    }
}

QStringList
Sites::decodeBilibili(const QString& s)
{
    QRegularExpression re_rid(qsl("\"room_info\"[^}]+?\"room_id\" *: *([0-9]+)"));
    QRegularExpression re_title("\"room_info\"[^}]+?\"title\" *: *\"([^\"]+)\"");
    QRegularExpression re_owner("\"base_info\"[^}]+?\"uname\" *: *\"([^\"]+)\"");
    QRegularExpression re_cover("\"room_info\"[^}]+?\"keyframe\" *: *\"([^\"]+)\"");
    QRegularExpression re_status("\"room_info\"[^}]+?\"live_status\" *: *([0-9]+)");

    QStringList ret;
    QRegularExpressionMatch match;
    match = re_rid.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_title.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_owner.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_cover.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_status.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    return ret;
}

QStringList
Sites::decodeHuya(const QString& s)
{
    QRegularExpression re_rid("\"roomInfo\":.+?\"lProfileRoom\":([0-9]+)");
    QRegularExpression re_title("\"roomInfo\":.+?\"sRoomName\":\"([^\"]+)\"");
    QRegularExpression re_owner("\"roomInfo\":.+?\"sNick\":\"([^\"]+)\"");
    QRegularExpression re_cover("\"roomInfo\":.+?\"sScreenshot\":\"([^\"]+)\"");
    QRegularExpression re_status("\"roomInfo\":.+?\"eLiveStatus\":([0-9])");
    QRegularExpression re_avatar("\"roomInfo\":.+?\"sAvatar180\":\"([^\"]+)\"");

    QStringList ret;
    QRegularExpressionMatch match;
    match = re_rid.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_title.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("No Title");
    }
    match = re_owner.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_cover.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        match = re_avatar.match(s);
        if (match.hasMatch()) {
            ret.append(match.captured(1));
        } else {
            ret.append("");
        }
    }
    match = re_status.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    return ret;
}

QStringList
Sites::decodeYoutube(const QString& ps)
{
    QRegularExpression re_status("(\"gridVideoRenderer\"((.(?!\"gridVideoRenderer\"))(?!\"style\":\"UPCOMING\"))+"
                                 "\"label\":\"(LIVE|LIVE NOW|PREMIERING NOW)\""
                                 "([\\s\\S](?!\"style\":\"UPCOMING\"))+?(\"gridVideoRenderer\"|</script>))");
    QRegularExpression re_rid("\"gridVideoRenderer\".+?\"channelId\":\"(.+?)\"");
    QRegularExpression re_title("\"gridVideoRenderer\".+?\"title\".+?\"text\":\"(.+?)(?<!\\\\)\"");
    QRegularExpression re_owner("\"gridVideoRenderer\".+?\"header\".+?\"title\":\"(.+?)\"");
    QRegularExpression re_cover("\"gridVideoRenderer\".+?\"thumbnail\".+?\"url\":\"(.+?)\"");
    QRegularExpression re_avatar("\"gridVideoRenderer\".+?\"avatar\".+?\"url\".+?\"url\":\"(.+?)\"");

    bool live = false;
    QString s;
    QStringList ret;
    QRegularExpressionMatch match;

    match = re_status.match(ps);
    if (match.hasMatch()) {
        live = true;
        s = match.captured(1);
    }

    match = re_rid.match(ps);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_title.match(s);
    if (match.hasMatch() && live == true) {
        ret.append(match.captured(1));
    } else {
        ret.append("No Title");
    }
    match = re_owner.match(ps);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_cover.match(s);
    if (match.hasMatch() && live == true) {
        ret.append(match.captured(1));
    } else {
        match = re_avatar.match(ps);
        if (match.hasMatch()) {
            ret.append(match.captured(1));
        } else {
            ret.append("");
        }
    }
    if (live == true) {
        ret.append("true");
    } else {
        ret.append("");
    }
    return ret;
}

QStringList
Sites::decodeTwitch(const QString& s)
{
    QRegularExpression re_rid("\"User\\}\\|\\{.+?\":.+?\"login\":\"(.+?)\"");
    QRegularExpression re_title("\"BroadcastSettings\\}\\|\\{.+?\":.+?\"title\":\"(.+?)\"");
    QRegularExpression re_owner("\"User\\}\\|\\{.+?\":.+?\"displayName\":\"(.+?)\"");
    QRegularExpression re_cover("\"Stream\\}\\|\\{.+?\":.+?\"previewImageURL\":\"(.+?)\"");
    QRegularExpression re_avatar("\"User\\}\\|\\{.+?\":.+?\"profileImageURL.+?\":\"(.+?)\"");
    QRegularExpression re_status("\"User\\}\\|\\{.+?\":.+?\"stream\":null");

    QStringList ret;
    bool status = true;
    QRegularExpressionMatch match;
    match = re_status.match(s);
    if (match.hasMatch()) {
        status = false;
    }
    match = re_rid.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_title.match(s);
    if (match.hasMatch() && status == true) {
        ret.append(match.captured(1));
    } else {
        ret.append("No Title");
    }
    match = re_owner.match(s);
    if (match.hasMatch()) {
        ret.append(match.captured(1));
    } else {
        ret.append("");
    }
    match = re_cover.match(s);
    if (match.hasMatch() && status == true) {
        ret.append(match.captured(1).replace("{width}", "320").replace("{height}", "180"));
    } else {
        match = re_avatar.match(s);
        if (match.hasMatch()) {
            ret.append(match.captured(1));
        } else {
            ret.append("");
        }
    }
    if (status) {
        ret.append("true");
    } else {
        ret.append("");
    }
    return ret;
}

inline QNetworkRequest
Sites::genRequest(QString url, bool is_phone, bool eng_only, bool open)
{
    QNetworkRequest qnr(url);
    qnr.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    //    qnr.setMaximumRedirectsAllowed(5);
    quint32 v = QRandomGenerator::global()->bounded(21) + 68;
    if (is_phone) {
        auto ua = qsl("Mozilla/5.0 (Android 10; Mobile; rv:%1.0) Gecko/%2.0 Firefox/%3.0").arg(v).arg(v).arg(v);
        qnr.setRawHeader(QByteArray("User-Agent"), ua.toLatin1());
    } else {
        auto ua = qsl("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:%1.0) Gecko/20100101 Firefox/%2.0").arg(v).arg(v);
        qnr.setRawHeader(QByteArray("User-Agent"), ua.toLatin1());
    }
    if (eng_only) {
        qnr.setRawHeader(QByteArray("accept-language"), QByteArray("en-US"));
    }
    if (open) {
        qnr.setRawHeader(QByteArray("qlp-open"), QByteArray().number(1));
    }
    return qnr;
}
