#include "clirecorder.h"

CLIRecorder::CLIRecorder()
    : QObject()
{
    mpvWidget = new MpvWidget(0, 0, true);
    mpvWidget->command(QStringList() << "loadfile" << "-");

    QStringList arguments = QCoreApplication::arguments();
    QStringList dmcPy;
    dmcPy.append("-c");
    dmcPy.append("exec(\"\"\"\\nimport time, sys\\nimport threading\\nfrom danmu import DanMuClient\\ndef pp(msg):\\n    print(msg)\\n    sys.stdout.flush()\\ndmc = DanMuClient(sys.argv[1])\\nif not dmc.isValid(): \\n    print('Url not valid')\\n    sys.exit()\\n@dmc.danmu\\ndef danmu_fn(msg):\\n    pp('[%s] %s' % (msg['NickName'], msg['Content']))\\ndmc.start(blockThread = True)\\n\"\"\")");
    dmcPy.append(arguments[2]);
    mProcess = new QProcess(this);
    connect(mProcess, &QProcess::readyReadStandardOutput, this, &CLIRecorder::readDanmaku);
    mProcess->start("python3", dmcPy);

    mTimer = new QTimer(this);
    mTimer->start(500);
    connect(mTimer, &QTimer::timeout, this, &CLIRecorder::checkVideoResolution);

    time.start();
}

CLIRecorder::~CLIRecorder()
{
    delete danmakuRecorder;
    danmakuRecorder = nullptr;
    mProcess->terminate();
    mProcess->waitForFinished(3000);
    mProcess->deleteLater();
}

void CLIRecorder::readDanmaku()
{
    while(!mProcess->atEnd())
    {
        QThread::msleep(10);
        QString newDanmaku(mProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(62, ' ');
//        danmakuPlayer->launchDanmaku(newDanmaku.remove(QRegExp("^\\[.*\\] ")));
        if(streamReady == true && (QCoreApplication::arguments().at(3) != "false"))
        {
            int availDChannel = getAvailDanmakuChannel();
            danmakuRecorder->danmaku2ASS(newDanmaku.remove(QRegExp("^\\[.*\\] ")), 13000, 24, availDChannel);
            danmakuTimeNodeSeq[availDChannel] = time.elapsed();
            danmakuTimeLengthSeq[availDChannel] = (newDanmaku.size() / 0.17)*25;
        }

    }
}

void CLIRecorder::checkVideoResolution()
{
//    qDebug() << mpvWidget->getProperty("video-params/w").toString();
    if(mpvWidget->getProperty("video-params/w").toString() != QString(""))
    {

        mTimer->start(5000);
        streamReady = true;
        if(QCoreApplication::arguments().at(3) != "false")
//            danmakuRecorder = new DanmakuRecorder(getProperty("video-params/w").toInt(), getProperty("video-params/h").toInt(), QCoreApplication::arguments().at(3));
            danmakuRecorder = new DanmakuRecorder(1280, 720, QCoreApplication::arguments().at(3));
    }
    else if(streamReady == true)
    {
        mTimer->stop();
        mTimer->deleteLater();
        QApplication::exit(2);
    }
}

int CLIRecorder::getAvailDanmakuChannel()
{
    int currentTime = time.elapsed();
    int i;
    for(i = 0; i < 24; i++)
    {
        if((currentTime - danmakuTimeNodeSeq[i]) > danmakuTimeLengthSeq[i])
            return i;
    }
    return 0;
}
