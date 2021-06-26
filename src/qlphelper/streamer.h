#ifndef STREAMER_H
#define STREAMER_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QQueue>
#include <QtCore>

class Streamer : public QObject
{
    Q_OBJECT

  public:
    explicit Streamer(QObject* parent = nullptr)
      : QObject(parent){

      };
    ~Streamer(){

    };

    enum State
    {
        Idle,
        Running,
        Closing
    } state = Idle;

    virtual void start() = 0;
    virtual void close() = 0;

  signals:
    void streamError();
    void streamStart();
};

class StreamerFlv : public Streamer
{
    Q_OBJECT
  public:
    explicit StreamerFlv(QString real_url, QString room_url, QString socket_path, QObject* parent = nullptr);
    ~StreamerFlv();

    void start() override;
    void close() override;

  private slots:
    void onProcFinished(int code, QProcess::ExitStatus es);
    void onProcStdout();
    void setSocket();

  private:
    QString real_url;
    QString room_url;
    QString stream_socket_path;
    QLocalServer* socket_server = nullptr;
    QLocalSocket* socket = nullptr;
    QProcess* proc = nullptr;

    void requestStream();
};

class StreamerHls : public Streamer
{
    Q_OBJECT
  public:
    explicit StreamerHls(QString real_url, QString socket_path, QObject* parent = nullptr);
    ~StreamerHls();

    void start() override;
    void close() override;

  private:
    QString real_url;
    QString stream_socket_path;
    QLocalServer* socket_server = nullptr;
    QLocalSocket* socket = nullptr;
    QNetworkAccessManager* nam = nullptr;
    qint64 hls_seg = -1;
    qint64 downloading_hls_seg = 0;
    qint64 last_downloaded_hls_seg_a = -1;
    qint64 last_downloaded_hls_seg_b = -2;
    int no_new_seg_time = 0;
    QQueue<QByteArray> hls_seg_queue;
    QMap<qint64, QPair<QByteArray, bool>> download_buf;
    QByteArray ua;

    void requestStream();
    void requestHlsManifest();
    void httpFinished(QNetworkReply* reply);
    void setSocket();
};

// using streamlink
class StreamerSl : public Streamer
{
    Q_OBJECT
  public:
    explicit StreamerSl(QString real_url, QString socket_path, int quality = 1, QObject* parent = nullptr);
    ~StreamerSl();

    void start() override;
    void close() override;

  private slots:
    void onProcStdout();
    void setSocket();
    void onProcFinished(int code, QProcess::ExitStatus es);

  private:
    QString real_url;
    QString stream_socket_path;
    QProcess* proc = nullptr;
    QLocalServer* socket_server = nullptr;
    QLocalSocket* socket = nullptr;
    int quality = 1;

    void requestStream();
};

#endif // STREAMER_H
