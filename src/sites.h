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

    void checkUrl(QString url, int cata, int num = 0);
    void checkUnverifiedUrl(QString url);
    void httpFinished(QNetworkReply *reply);


signals:
    void roomDecoded(int cata, QString url, QString title, QString owner, QString cover, int status, int num);
    void urlVerified(QString url);

private:
    QNetworkAccessManager *nam = nullptr;

    QStringList decodeDouyu(const QByteArray &s);
    QStringList decodeBilibili(const QString &s);
    QStringList decodeHuya(const QString &s);
    QStringList decodeYoutube(const QString &ps);
    QStringList decodeTwitch(const QString &s);
    QNetworkRequest genRequest(QString url, bool is_unverified, bool is_phone, int cata = 0, int num = 0, bool eng_only = false);
};

#endif // SITES_H
