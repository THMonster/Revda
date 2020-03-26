#ifndef BILIBILI_H
#define BILIBILI_H

#include <QtCore>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

struct DanmakuChannel
{
//    double duration;
    int length;
    double begin_pts;
};

class Bilibili : public QObject
{
    Q_OBJECT
public:
    explicit Bilibili(QObject *parent = nullptr);
    ~Bilibili();

    void run(QString av_num, QString part = "");
    void genAss();
    void setRes();
    int getDankamuDisplayLength(QString dm, int fontsize);
    int getAvailDMChannel(double time_start, int len);
    void httpFinished(QNetworkReply *reply);

private:
    double speed = 8.0; // duration of single danmaku in second
    int res_x = 1920;
    int res_y = 1080;
    QStringList real_url;
    QString title;
    QFile *ass_file = nullptr;
    QMap<double, QPair<QString, int>> danmaku_map;
    DanmakuChannel danmaku_channel[30];
    QNetworkAccessManager *nam = nullptr;
};

#endif // BILIBILI_H
