#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "sites.h"

Sites::Sites(QObject *parent) : QObject(parent)
{
    nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished, this, &Sites::httpFinished);
}

void Sites::checkUrl(QString url, int cata, int num)
{
    auto sl = url.split('-');
    if (sl[0] == "do") {
        QNetworkRequest qnr("https://www.douyu.com/betard/" + sl[1]);
        qnr.setRawHeader(QByteArray("qlp-cata"), QByteArray().number(cata));
        qnr.setRawHeader(QByteArray("qlp-order"), QByteArray().number(num));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Mobile Safari/537.36"));
        nam->get(qnr);
    } else if (sl[0] == "bi") {
        QNetworkRequest qnr("https://api.live.bilibili.com/xlive/web-room/v1/index/getInfoByRoom?room_id=" + sl[1]);
        qnr.setRawHeader(QByteArray("qlp-cata"), QByteArray().number(cata));
        qnr.setRawHeader(QByteArray("qlp-order"), QByteArray().number(num));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36"));
        nam->get(qnr);
    } else if (sl[0] == "hu") {
        QNetworkRequest qnr("https://m.huya.com/" + sl[1]);
        qnr.setRawHeader(QByteArray("qlp-cata"), QByteArray().number(cata));
        qnr.setRawHeader(QByteArray("qlp-order"), QByteArray().number(num));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Mobile Safari/537.36"));
        nam->get(qnr);
    } else if (sl[0] == "yt") {
        QNetworkRequest qnr("https://www.youtube.com/channel/" +  url.mid(3) + "/videos");
        qnr.setRawHeader(QByteArray("qlp-cata"), QByteArray().number(cata));
        qnr.setRawHeader(QByteArray("qlp-order"), QByteArray().number(num));
        qnr.setRawHeader(QByteArray("accept-language"), QByteArray("en-US"));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36"));
        nam->get(qnr);
    } else if (sl[0] == "ytv") {
        QNetworkRequest qnr("https://www.youtube.com/c/" + url.mid(4) + "/videos");
        qnr.setRawHeader(QByteArray("qlp-cata"), QByteArray().number(cata));
        qnr.setRawHeader(QByteArray("qlp-order"), QByteArray().number(num));
        qnr.setRawHeader(QByteArray("accept-language"), QByteArray("en-US"));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36"));
        nam->get(qnr);
    }
}

void Sites::checkUnverifiedUrl(QString url)
{
    auto sl = url.split('-');
    if (sl[0] == "do") {
        QNetworkRequest qnr("https://www.douyu.com/betard/" + sl[1]);
        qnr.setRawHeader(QByteArray("qlp-unverified"), QByteArray("true"));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Mobile Safari/537.36"));
        nam->get(qnr);
    } else if (sl[0] == "bi") {
        QNetworkRequest qnr("https://api.live.bilibili.com/xlive/web-room/v1/index/getInfoByRoom?room_id=" + sl[1]);
        qnr.setRawHeader(QByteArray("qlp-unverified"), QByteArray("true"));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36"));
        nam->get(qnr);
    } else if (sl[0] == "hu") {
        QNetworkRequest qnr("https://m.huya.com/" + sl[1]);
        qnr.setRawHeader(QByteArray("qlp-unverified"), QByteArray("true"));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Mobile Safari/537.36"));
        nam->get(qnr);
    } else if (sl[0] == "yt") {
        QNetworkRequest qnr("https://www.youtube.com/channel/" + url.mid(3) + "/videos");
        qnr.setRawHeader(QByteArray("qlp-unverified"), QByteArray("true"));
        qnr.setRawHeader(QByteArray("accept-language"), QByteArray("en-US"));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36"));
        nam->get(qnr);
    } else if (sl[0] == "ytv") {
        QNetworkRequest qnr("https://www.youtube.com/c/" + url.mid(4) + "/videos");
        qnr.setRawHeader(QByteArray("qlp-unverified"), QByteArray("true"));
        qnr.setRawHeader(QByteArray("accept-language"), QByteArray("en-US"));
        qnr.setRawHeader(QByteArray("user-agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36"));
        nam->get(qnr);
    }
}

void Sites::httpFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error: " << reply->error();
        reply->deleteLater();
        return;
    }
    if (reply->request().url().toString().contains("douyu")) {
        int cata = reply->request().rawHeader("qlp-cata") == "1" ? 1 : 0;
        int num = reply->request().rawHeader("qlp-order").toInt();
        QStringList sl = decodeDouyu(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://www.douyu.com/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        if (reply->request().rawHeader("qlp-unverified") == "true") {
            emit urlVerified(url);
            return;
        }
        emit roomDecoded(cata, url, sl[1], sl[2], sl[3], sl[4] == "1" ? 1 : 0, num);
    } else if (reply->request().url().toString().contains("bilibili")) {
        int cata = reply->request().rawHeader("qlp-cata") == "1" ? 1 : 0;
        int num = reply->request().rawHeader("qlp-order").toInt();
        QStringList sl = decodeBilibili(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://live.bilibili.com/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        if (reply->request().rawHeader("qlp-unverified") == "true") {
            emit urlVerified(url);
            return;
        }
        emit roomDecoded(cata, url, sl[1], sl[2], sl[3], sl[4] == "1" ? 1 : 0, num);
    } else if (reply->request().url().toString().contains("huya")) {
        int cata = reply->request().rawHeader("qlp-cata") == "1" ? 1 : 0;
        int num = reply->request().rawHeader("qlp-order").toInt();
        QStringList sl = decodeHuya(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://www.huya.com/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        if (reply->request().rawHeader("qlp-unverified") == "true") {
            emit urlVerified(url);
            return;
        }
        emit roomDecoded(cata, url, sl[1], sl[2], sl[3], sl[4] == "true" ? 1 : 0, num);
    } else if (reply->request().url().toString().contains("youtube.com/c")) {
        QStringList sl = decodeYoutube(reply->readAll());
        QString url;
        if (!sl[0].isEmpty()) {
            url = "https://www.youtube.com/channel/" + sl[0];
        } else {
            reply->deleteLater();
            return;
        }
        if (reply->request().rawHeader("qlp-unverified") == "true") {
            emit urlVerified(url);
            return;
        }
        int cata = reply->request().rawHeader("qlp-cata") == "1" ? 1 : 0;
        int num = reply->request().rawHeader("qlp-order").toInt();
        emit roomDecoded(cata, url, sl[1], sl[2], sl[3], sl[4] == "true" ? 1 : 0, num);
    }

    reply->deleteLater();
    return;
}

QStringList Sites::decodeDouyu(const QByteArray &s)
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
            (tmp.value("show_status").toInt(2) == 1 && tmp.value("videoLoop").toInt(1) == 0) ?
                        ret.append("1") :
                        ret.append("");
        }
    }
//    qDebug() << ret;
    if (ret.length() != 5) {
        return QStringList() << "" << "" << "" << "" << "";
    } else {
        return ret;
    }
}

QStringList Sites::decodeBilibili(const QString &s)
{
    QRegularExpression re_rid("\"room_info\"[^}]+?\"room_id\" *: *([0-9]+)");
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

QStringList Sites::decodeHuya(const QString &s)
{
    QRegularExpression re_rid("class=\"roomid\">[^0-9]*([0-9]+)[^0-9]+");
    QRegularExpression re_title("liveRoomName *= *'([^']+)'");
    QRegularExpression re_owner("ANTHOR_NICK *= *'([^']+)'");
    QRegularExpression re_cover("picURL *= *'([^']+)'");
    QRegularExpression re_status("ISLIVE *= *(true)");

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

// owner status title cover
QStringList Sites::decodeYoutube(const QString &ps)
{
    qDebug() << ps;
    QRegularExpression re_data("\"gridVideoRenderer\"[\\s\\S]+?</script>");
    QRegularExpression re_data1("(\"gridVideoRenderer\"(.(?!\"gridVideoRenderer\"))+\"label\":\"(LIVE|LIVE NOW)\"[\\s\\S]+?</script>)");
    QRegularExpression re_rid("\"gridVideoRenderer\".+?\"channelId\":\"(.+?)\"");
    QRegularExpression re_title("\"gridVideoRenderer\".+?\"title\".+?\"text\":\"(.+?)\"");
    QRegularExpression re_owner("\"gridVideoRenderer\".+?\"header\".+?\"title\":\"(.+?)\"");
    QRegularExpression re_cover("\"gridVideoRenderer\".+?\"thumbnail\".+?\"url\":\"(.+?)\"");
    QRegularExpression re_avatar("\"gridVideoRenderer\".+?\"avatar\".+?\"url\".+?\"url\":\"(.+?)\"");
    QRegularExpression re_status("\"gridVideoRenderer\".+?\"label\":\"(LIVE|LIVE NOW)\"");

    bool live = false;
    QString s;
    QStringList ret;
    QRegularExpressionMatch match;

    match = re_status.match(ps);
    if (match.hasMatch()) {
        live = true;
        match = re_data1.match(ps);
        if (match.hasMatch()) {
            s = match.captured(1);
        }
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
