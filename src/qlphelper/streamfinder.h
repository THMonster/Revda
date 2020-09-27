#ifndef STREAMFINDER_H
#define STREAMFINDER_H

#include <QObject>
#include <QtCore>

#include "streamer.h"

class StreamFinder : public QObject
{
    Q_OBJECT
public:
    explicit StreamFinder(QString room_url, QString stream_socket, QObject *parent = nullptr);
    ~StreamFinder();

    void start();
    void restart();
    void stop();
    void setQuality(int q);

signals:
    void titleMatched(QString title);
    void streamError();
    void streamStart();
    void ready(QString title, int flag);


private:
    QString room_url;
    QString stream_socket;
    QString real_url;
    QString title;
    QProcess *proc = nullptr;
    int proc_id = 0;
    int offline_counter = 0;
    Streamer *streamer = nullptr;
    int quality = 1;

    void startRequest();
    void slotSfpyResponse(int code, QProcess::ExitStatus es);
    void startStreamer();
};

#endif // STREAMFINDER_H
