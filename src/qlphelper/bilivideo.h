#ifndef BILIVIDEO_H
#define BILIVIDEO_H

#include <QtCore>
#include <QLocalSocket>
#include <QLocalServer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "qlpcmdparser.h"

namespace BV {

struct DanmakuChannelB
{
//    double duration;
    int length;
    double begin_pts;
};

class MpvControl : public QObject
{
    Q_OBJECT
public:
    explicit MpvControl(QObject *parent = nullptr);
    ~MpvControl();

public slots:
    void start();
    void loadVideo(QString edl_url, QString ass_file_path, QString title);
    void readMpvSocket();

signals:
    void requestReload();
    void playFinished();
    void jumpReceived(int n);
    void prevReceived();
    void nextReceived();
    void fileLoaded();

private:
    QString ass_path;
    QString mpv_socket_path;
    QLocalSocket* mpv_socket = nullptr;
    QProcess* mpv_proc;
};

class BiliVideo : public QObject
{
    Q_OBJECT
public:
    explicit BiliVideo(QObject *parent = nullptr);
    ~BiliVideo();

    void run(QString url);
    void genAss();
    void setRes();
    int getDankamuDisplayLength(QString dm, int fontsize);
    int getAvailDMChannel(double time_start, int len);
    void slotHttpDMXml();
    void slotHttpVideoInfo();
    void genEDLUrl();
    void downloadVideo();
    void setSavedFilePath(QString path);
    void startSegFifoProc();
    void requestRealUrl(QString url);
    void playPage(int p);
    void autoNextPage();
    void goPrevPage();
    void goNextPage();

signals:
    void dlFinished();

private:
    double speed = 8.0; // duration of single danmaku in second
    int res_x = 1920;
    int res_y = 1080;
    bool hevc = false;
    MpvControl *mpv = nullptr;
    QStringList real_url;
    QString edl_url;
    QString title;
    QString cookie;
    QFile *ass_file = nullptr;
    QMap<double, QPair<QString, int>> danmaku_map;
    DanmakuChannelB danmaku_channel[30];
    QNetworkAccessManager *nam = nullptr;
    QProcess *mpv_proc = nullptr;
    QFile *saved_file = nullptr;
    QFile *merge_file = nullptr;
    QList<QFile*> seg_file_list;
    QVector<QString> pages;
    int current_page = 1;
    QString base_url;
    QNetworkReply *reply_dm = nullptr;
    QNetworkReply *reply_info = nullptr;
};

}
#endif // BILIVIDEO_H
