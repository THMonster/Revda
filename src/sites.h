#ifndef SITES_H
#define SITES_H

#include <QObject>
#include <QtGui>
#include <QNetworkAccessManager>

class Sites : public QObject
{
    Q_OBJECT
public:
    explicit Sites(QObject *parent = nullptr);

    void checkUrl(QString url, bool open = false);
//    void checkUnverifiedUrl(QString url);
    void httpFinished(QNetworkReply *reply);


signals:
    void roomDecoded(QString url, QString title, QString owner, QString cover, int status, bool open);
    void urlVerified(QString url);

private:
    QNetworkAccessManager *nam = nullptr;

    QStringList decodeDouyu(const QByteArray &s);
    QStringList decodeBilibili(const QString &s);
    QStringList decodeHuya(const QString &s);
    QStringList decodeYoutube(const QString &ps);
    QStringList decodeTwitch(const QString &s);
    QNetworkRequest genRequest(QString url, bool is_phone, bool eng_only = false, bool open = false);
};

#endif // SITES_H
