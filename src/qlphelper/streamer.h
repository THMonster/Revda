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
    explicit Streamer(QObject *parent = nullptr)
        : QObject(parent)
    {

    };
    ~Streamer()
    {

    };

    enum State {
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
    explicit StreamerFlv(QString real_url, QString socket_path, QObject *parent = nullptr);
    ~StreamerFlv();

    void start() override;
    void close() override;

private slots:
    void onStreamData();
    void onHttpFinished();

private:
    QString real_url;
    QString stream_socket_path;
    QLocalServer* socket_server = nullptr;
    QLocalSocket* socket = nullptr;
    QNetworkAccessManager* nam = nullptr;
    QNetworkReply* reply_stream = nullptr;

    void setSocket();
    void requestStream();
};

class StreamerHls : public Streamer
{
    Q_OBJECT
public:
    explicit StreamerHls(QString real_url, QString socket_path, QObject *parent = nullptr);
    ~StreamerHls();

    void start() override;
    void close() override;

private slots:
    void onProcStdout();
    void onProcFinished(int code, QProcess::ExitStatus es);

private:
    QString real_url;
    QString stream_socket_path;
    QProcess *proc = nullptr;

    void requestStream();
};

// using streamlink
class StreamerSl : public Streamer
{
    Q_OBJECT
public:
    explicit StreamerSl(QString real_url, QString socket_path, QObject *parent = nullptr);
    ~StreamerSl();

    void start() override;
    void close() override;

private slots:
    void onProcStdout();
    void onProcFinished(int code, QProcess::ExitStatus es);

private:
    QString real_url;
    QString stream_socket_path;
    QProcess *proc = nullptr;

    void requestStream();
};

#endif // STREAMER_H
