#ifndef STREAMER_H
#define STREAMER_H

#include <QObject>
#include <QtCore>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLocalServer>
#include <QLocalSocket>


class Streamer : public QObject
{
    Q_OBJECT
public:
    explicit Streamer(QString room_url, QString stream_socket, QObject *parent = nullptr);
    ~Streamer();


signals:
    void streamError();
    void streamStart();
    void titleMatched(QString title);

public slots:
    void requestStreamHLS();
    void onStreamDataHLS();
    void requestStream();
    void requestRealUrl();
    void setRealUrl(int code, QProcess::ExitStatus es);
    void onStreamData();
    void onHttpFinished();
    void start();
    void restart();
    void stop();
    void setSocket();
    void connectToFFmpeg();

private:
    QString room_url;
    QString real_url;
    QString stream_socket_path;
    QString ffmpeg_server_path;
    QLocalServer* socket_server = nullptr;
    QLocalSocket* socket = nullptr;
    QLocalSocket* socket_hls = nullptr;
    QNetworkAccessManager* nam = nullptr;
    QNetworkReply* reply_stream = nullptr;
    QProcess* ykdl_process = nullptr;
    QProcess* hls_ffmpeg_process = nullptr;
    bool on_streaming = false;
    int offline_counter = 0;
    int ykdl_id = 0;
    bool is_hls = false;
    bool terminate_ffmpeg = false;
    bool manual_restart_flag = true;

};

#endif // STREAMER_H
