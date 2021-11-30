#ifndef STREAMER_H
#define STREAMER_H

#include <QEnableSharedFromThis>
#include <QLocalServer>
#include <QLocalSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QQueue>
#include <QtCore>

#include "../Binding.h"

class Streamer
  : public QObject
  , public QEnableSharedFromThis<Streamer>
{
    Q_OBJECT

  public:
    explicit Streamer(){};
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

  private:
    QString real_url;
    QString room_url;
    QString stream_socket_path;
    QSharedPointer<QLivePlayerLib> qlp_lib;

    void setError();
    void qlp_run_streamer();
    void qlp_check_streamer_loading();

  signals:
    void qlp_streamer_finished();
    void qlp_streamer_stream_started();
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
    QSharedPointer<QLivePlayerLib> qlp_lib;

    void setError();
    void qlp_run_streamer();
    void qlp_check_streamer_loading();

  signals:
    void qlp_streamer_finished();
    void qlp_streamer_stream_started();
};

class StreamerDash : public Streamer
{
    Q_OBJECT
  public:
    explicit StreamerDash(QString real_url, QString socket_path, QObject* parent = nullptr);
    ~StreamerDash();

    void start() override;
    void close() override;

  private:
    QString real_url;
    QString stream_socket_path;
    QSharedPointer<QLivePlayerLib> qlp_lib;

    void setError();
    void qlp_run_streamer();
    void qlp_check_streamer_loading();

  signals:
    void qlp_streamer_finished();
    void qlp_streamer_stream_started();
};

#endif // STREAMER_H
