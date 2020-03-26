#ifndef DANMAKURECORDER_H
#define DANMAKURECORDER_H
#include <QTime>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QRegExp>

class DanmakuRecorder
{

public:
    DanmakuRecorder(int width, int height, QString fileName);
    void initDRecorder();
    void addASS(QString assSpeaker, QString assText, int durationMs, int x1, int y1, int x2, int y2);
    void danmaku2ASS(QString assSpeaker, QString assText, int duration, double length, int channelNum, int channel);
    void pause();
    void resume();
    void start();
private:
    QString fileName;
    QTime startTime;
    QString assHeaderTemplate;
    QString assDanmakuTemplate;
    int videoWidth = 0;
    int videoHeight = 0;
    bool paused = false;
    bool started = false;
    int pausedStartTimeMs = 0;
    int pausedTimeMs = 0;
};

#endif // DANMAKURECORDER_H
