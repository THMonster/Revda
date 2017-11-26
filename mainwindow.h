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

class MpvWidget;
class QSlider;
class QPushButton;
class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public Q_SLOTS:
    void openMedia();
    void seek(int pos);
    void pauseResume();

    void readDanmaku();

private Q_SLOTS:
    void setSliderRange(int duration);
private:
    DanmakuPlayer *danmakuPlayer;
    QSlider *m_slider;
    QPushButton *m_openBtn;
    QPushButton *m_playBtn;
    QLabel* mLabel;
    QProcess* mProcess;
};

#endif // MainWindow_H
