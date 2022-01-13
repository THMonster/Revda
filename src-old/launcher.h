#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QObject>
#include <QtCore>
#include <QLocalServer>
#include <QLocalSocket>
#include "mainwindow.h"

class Launcher : public QObject
{
    Q_OBJECT
public:
    explicit Launcher(QObject *parent = nullptr);
    ~Launcher();

    void start();

public slots:
    void socketConnected();
    void socketReadyRead();
    void socketError(QLocalSocket::LocalSocketError error);
    void newInstanceConnected();

signals:

private:
    bool second_instance = false;
    std::unique_ptr<MainWindow> w;
    QString local_socket_name;
    QLocalSocket local_socket;
    QLocalServer local_server;

    void startMainWindow();
    void activate();


};

#endif // LAUNCHER_H
