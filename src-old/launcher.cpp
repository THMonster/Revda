#include "launcher.h"

Launcher::Launcher(QObject *parent) : QObject(parent)
{
    local_socket_name = QCryptographicHash::hash(QSysInfo::bootUniqueId(), QCryptographicHash::Md5).toHex();
}

Launcher::~Launcher()
{

}

void Launcher::start()
{
    connect(&local_socket, &QLocalSocket::connected, this, &Launcher::socketConnected);
    connect(&local_socket, &QLocalSocket::readyRead, this, &Launcher::socketReadyRead);
    connect(&local_socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), this ,&Launcher::socketError);
    connect(&local_server, &QLocalServer::newConnection, this, &Launcher::newInstanceConnected);
    local_socket.connectToServer(QStringLiteral("/tmp/qlp-%1").arg(local_socket_name));
}

void Launcher::socketConnected()
{
    second_instance = true;
    local_socket.write(QStringLiteral("show").toLatin1());
}

void Launcher::socketReadyRead()
{
    if(local_socket.readAll().startsWith("showed")) {
        qDebug().noquote() << QStringLiteral("Response received, exiting...");
        QCoreApplication::quit();
    }
}

void Launcher::socketError(QLocalSocket::LocalSocketError error)
{
//    if (second_instance) {
//        return QCoreApplication::quit();
//    }

    if (error == QLocalSocket::ServerNotFoundError) {
        qDebug().noquote() << QStringLiteral("No instance found, starting mainwindow...");
    } else {
        qDebug().noquote() << QStringLiteral("Socket connect failed: %1, remove existing server file...").arg(error);
        QLocalServer::removeServer(QStringLiteral("/tmp/qlp-%1").arg(local_socket_name));
    }
    local_socket.close();
    if (!local_server.listen(QStringLiteral("/tmp/qlp-%1").arg(local_socket_name))) {
        qDebug().noquote() << QStringLiteral("Listen local socket failed.");
        QCoreApplication::quit();
        return;
    }
    startMainWindow();
}

void Launcher::newInstanceConnected()
{
    for (auto client = local_server.nextPendingConnection(); client; client = local_server.nextPendingConnection()) {
        connect(client, &QLocalSocket::readyRead, [=]() {
            if (client->readAll().startsWith("show")) {
                qDebug().noquote() << QStringLiteral("Request received, activating...");
                activate();
                client->write(QStringLiteral("showed").toLatin1());
            }
        });
    }
}

void Launcher::startMainWindow()
{
    if (w.get() == nullptr) {
        qDebug().noquote() << QStringLiteral("Starting mainwindow...");
        w = std::make_unique<MainWindow>();
        w->setWindowTitle(QStringLiteral("QLivePlayer"));
        w->show();
    }
}

void Launcher::activate()
{
    if (w.get() != nullptr) {
        w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
        w->setVisible(true);
        w->raise();
        w->activateWindow();
    }
}
