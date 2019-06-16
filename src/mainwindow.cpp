#include "mainwindow.h"
#include "mpvwidget.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QLabel>
#include <QFile>
#include <QUuid>

MainWindow::MainWindow(QStringList args,QWidget *parent) : QWidget(parent)
{
    this->args = args;
    suspendPM();
    getStreamUrl();
//    startStreamlinkProcess();
//    qDebug() << args;
    danmakuPlayer = new DanmakuPlayer(args, this, nullptr);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(danmakuPlayer);

    setLayout(vl);
    danmakuPlayer->command(QStringList() << "loadfile" << stream_url);

    connect(danmakuPlayer, &DanmakuPlayer::refreshStream, this, &MainWindow::refreshDanmakuPlayer);
}

MainWindow::~MainWindow()
{
    resumePM();
//    QProcess::execute("rm " + namedPipe);
//    streamLinkProcess->terminate();
//    streamLinkProcess->waitForFinished(3000);
//    streamLinkProcess->deleteLater();
}

void MainWindow::startStreamlinkProcess()
{
//    namedPipe = "/tmp/qlivesplayer-" + QUuid::createUuid().toString();
//    QProcess::execute("mkfifo " + namedPipe);
//    streamLinkProcess = new QProcess(this);
//    if(args.at(2) == QString("false")) {
//        streamLinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O > " + namedPipe + "\"");
//    } else
//        streamLinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O | tee " + args.at(2) + QString(".%1").arg(videoPart++) + " > " + namedPipe + "\"");
//    if(!streamLinkProcess->waitForStarted())
//        QApplication::exit(1);
}

void MainWindow::getStreamUrl()
{
    QProcess p;
    p.start("ykdl", QStringList() << "-i" << args.at(0));
    p.waitForStarted();
    p.waitForFinished();
    QRegularExpression re("^(http.+)$");
    while(!p.atEnd()) {
        QRegularExpressionMatch match = re.match(p.readLine());
        if (match.hasMatch()) {
             stream_url = match.captured(1);
             return;
        }
    }


}

void MainWindow::refreshDanmakuPlayer()
{
    danmakuPlayer->command(QStringList() << "stop");
    getStreamUrl();
    danmakuPlayer->command(QStringList() << "loadfile" << stream_url);
}

void MainWindow::openMedia()
{
    QString file = QFileDialog::getOpenFileName(0, "Open a video");
    if (file.isEmpty())
        return;
//    m_mpv->command(QStringList() << "loadfile" << file);
    danmakuPlayer->command(QStringList() << "loadfile" << "-");
}

void MainWindow::seek(int pos)
{
    danmakuPlayer->command(QVariantList() << "seek" << pos << "absolute");
}

void MainWindow::pauseResume()
{
    const bool paused = danmakuPlayer->getProperty("pause").toBool();
    danmakuPlayer->setProperty("pause", !paused);
}

void MainWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
}

void MainWindow::suspendPM()
{
    if (QString(std::getenv("XDG_CURRENT_DESKTOP")) == "KDE") {
        QDBusInterface kpm("org.kde.Solid.PowerManagement.PolicyAgent", "/org/kde/Solid/PowerManagement/PolicyAgent", "org.kde.Solid.PowerManagement.PolicyAgent");
        QDBusReply<uint> r = kpm.call("AddInhibition", static_cast<uint>(4), "QLivePlayer", "Playing stream");
        if (r.isValid()) {
            pm_reply = r.value();
        } else {
            qDebug() << "Bad DBus call!";
        }
    } else {
        QDBusInterface pm("org.freedesktop.PowerManagement.Inhibit", "/org/freedesktop/PowerManagement/Inhibit", "org.freedesktop.PowerManagement.Inhibit");
        QDBusReply<uint> r = pm.call("Inhibit", "QLivePlayer", "Playing stream");
        if (r.isValid()) {
            pm_reply = r.value();
        } else {
            qDebug() << "Bad DBus call!";
        }
    }
}

void MainWindow::resumePM()
{
    if (QString(std::getenv("XDG_CURRENT_DESKTOP")) == "KDE") {
        QDBusInterface kpm("org.kde.Solid.PowerManagement.PolicyAgent", "/org/kde/Solid/PowerManagement/PolicyAgent", "org.kde.Solid.PowerManagement.PolicyAgent");
        kpm.call("ReleaseInhibition", pm_reply);
    } else {
        QDBusInterface pm("org.freedesktop.PowerManagement.Inhibit", "/org/freedesktop/PowerManagement/Inhibit", "org.freedesktop.PowerManagement.Inhibit");
        pm.call("UnInhibit", pm_reply);
    }
}
