#ifndef BILIVIDEO_H
#define BILIVIDEO_H

#include <QtCore>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

struct DanmakuChannelB
{
//    double duration;
    int length;
    double begin_pts;
};

class BiliVideo : public QObject
{
    Q_OBJECT
public:
    explicit BiliVideo(QObject *parent = nullptr);
    ~BiliVideo();

    void run(QString url, QString part = "");
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
    QString cookie;
    QFile *ass_file = nullptr;
    QMap<double, QPair<QString, int>> danmaku_map;
    DanmakuChannelB danmaku_channel[30];
    QNetworkAccessManager *nam = nullptr;
    QProcess *mpv_proc = nullptr;
};

#endif // BILIVIDEO_H
