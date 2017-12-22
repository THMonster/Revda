#include "danmakurecorder.h"
#include <QDebug>

DanmakuRecorder::DanmakuRecorder(int width, int height, QString fileName)
{
    videoHeight = height;
    videoWidth = width;
    this->fileName = fileName;
//    startTime.start();
    initDRecorder();
}

void DanmakuRecorder::initDRecorder()
{
    QFile file(":/ass/ass_template");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    assHeaderTemplate = in.readAll();
    file.close();

    assHeaderTemplate.replace(QRegExp("<qlp_width>"), QString().number(videoWidth));
    assHeaderTemplate.replace(QRegExp("<qlp_height>"), QString().number(videoHeight));

//    file(fileName);
    QFile file1(fileName);
    if (!file1.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    QTextStream out(&file1);
    out.setCodec("UTF-8");
    out << assHeaderTemplate;
    out.flush();
    file1.close();

    assDanmakuTemplate = "Dialogue: 0,<qlp_startTime>,<qlp_endTime>,Default,<qlp_speaker>,0,0,0,,{\\move(<qlp_x1>,<qlp_y1>,<qlp_x2>,<qlp_y2>)}<qlp_text>";
}

void DanmakuRecorder::addASS(QString assSpeaker, QString assText, int durationMs, int x1, int y1, int x2, int y2)
{
    QString tempTime;
    QTime time(0,0);
    QString temp(assDanmakuTemplate);
    time = time.addMSecs(startTime.elapsed() - pausedTimeMs);
    tempTime = time.toString("hh:mm:ss.zzz");
    tempTime.chop(1);
    temp.replace(QRegExp("<qlp_startTime>"), tempTime);
    time = time.addMSecs(durationMs);
    tempTime = time.toString("hh:mm:ss.zzz");
    tempTime.chop(1);
    temp.replace(QRegExp("<qlp_endTime>"), tempTime);
    temp.replace(QRegExp("<qlp_speaker>"), assSpeaker);
    temp.replace(QRegExp("<qlp_x1>"), QString().number(x1));
    temp.replace(QRegExp("<qlp_y1>"), QString().number(y1));
    temp.replace(QRegExp("<qlp_x2>"), QString().number(x2));
    temp.replace(QRegExp("<qlp_y2>"), QString().number(y2));
    temp.replace(QRegExp("<qlp_text>"), assText);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
        return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << temp + "\n";
    out.flush();
    file.close();
}

void DanmakuRecorder::danmaku2ASS(QString assSpeaker, QString assText, int duration, double length, int channelNum, int channel)
{
//    qDebug() << QString().number(channel);
    int x1, y1, x2, y2;
    y1 = channel * (videoHeight/channelNum);
//    qDebug() << y1;
    y2 = y1;
    x1 = videoWidth;
    x2 = -length;
    addASS(assSpeaker, assText, duration, x1, y1, x2, y2);
}

void DanmakuRecorder::pause()
{
    if(paused == false)
    {
        pausedStartTimeMs = startTime.elapsed();
        paused = true;
    }
}

void DanmakuRecorder::resume()
{
    if(paused == true)
    {
        pausedTimeMs += startTime.elapsed() - pausedStartTimeMs;
        paused = false;
    }
}

void DanmakuRecorder::start()
{
    if (started != true) {
        startTime.start();
        started = true;
    }
}
