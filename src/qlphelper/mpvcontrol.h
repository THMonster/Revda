#ifndef MPVCONTROL_H
#define MPVCONTROL_H

#include <QtCore>
#include <QLocalSocket>
#include <QLocalServer>

#include "qlpcmdparser.h"

class MpvControl : public QObject
{
    Q_OBJECT
public:
    explicit MpvControl(QFile* ff2mpv_fifo, QString record_file = "", QObject *parent = nullptr);
    ~MpvControl();

public slots:
    void start();
    void restart();
    void readMpvSocket();
    void setTitle(QString title);

signals:
    void requestReload();
    void reloaded();
    void resFetched(int w, int h);
    void onQuality(int q);
    void onFont(double fs, double fa);
    void onFontScaleDelta(double delta);
    void onToggleNick();
    void onSpeed(int speed);

private:
    QString room_title;
    QFile* ff2mpv_fifo = nullptr;
    QString mpv_socket_path;
    QString record_file;
    QLocalSocket* mpv_socket = nullptr;
    QProcess* mpv_proc;
    bool no_window;
    int record_cnt = 0;

    QString genRecordFileName();


};

#endif // MPVCONTROL_H
