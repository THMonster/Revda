#ifndef MainWindow_H
#define MainWindow_H

#include <QtWidgets/QWidget>
#include <QLabel>
#include <QProcess>
#include <QString>
#include <QApplication>
#include <QStringList>
#include <QRegExp>
#include "mpvwidget.h"
#include <QTimer>
#include <QThread>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

class MpvWidget;
class QSlider;
class QPushButton;
class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QStringList args, QWidget *parent = 0);
    ~MainWindow();
    void startStreamlinkProcess();
    void getStreamUrl();
    void refreshDanmakuPlayer();

public Q_SLOTS:
    void openMedia();
    void seek(int pos);
    void pauseResume();


private Q_SLOTS:
    void setSliderRange(int duration);
private:
    int videoPart = 1;
    QTimer* readDanmakuTimer;
    DanmakuPlayer *danmakuPlayer;
    QSlider *m_slider;
    QPushButton *m_openBtn;
    QPushButton *m_playBtn;
    QProcess* dmcPyProcess;
//    QProcess* streamLinkProcess;
    QStringList args;
//    QString namedPipe;
    QString stream_url;
    uint pm_reply = 0;

    void suspendPM();
    void resumePM();
};

#endif // MainWindow_H
