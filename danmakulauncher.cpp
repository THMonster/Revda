#include "danmakulauncher.h"

DanmakuLauncher::DanmakuLauncher(QStringList args, DanmakuPlayer *parent)
        :QObject(0)
{
    this->args = args;
    dmp = parent;
}

DanmakuLauncher::~DanmakuLauncher()
{
    dmcPyProcess->terminate();
    dmcPyProcess->waitForFinished(3000);
    dmcPyProcess->deleteLater();
}

void DanmakuLauncher::initDmcPy()
{
    time.start();
    QStringList dmcPy;
    dmcPy.append("-c");
    dmcPy.append("exec(\"\"\"\\nimport time, sys\\nimport threading\\nfrom danmu import DanMuClient\\ndef pp(msg):\\n    print(msg)\\n    sys.stdout.flush()\\ndmc = DanMuClient(sys.argv[1])\\nif not dmc.isValid(): \\n    print('Url not valid')\\n    sys.exit()\\n@dmc.danmu\\ndef danmu_fn(msg):\\n    pp('[%s] %s' % (msg['NickName'], msg['Content']))\\ndmc.start(blockThread = True)\\n\"\"\")");
    dmcPy.append(args.at(0));
    dmcPyProcess = new QProcess(this);
    launchDanmakuTimer = new QTimer(this);
    launchDanmakuTimer->start(200);
    connect(launchDanmakuTimer, &QTimer::timeout, this, &DanmakuLauncher::launchDanmaku);
    dmcPyProcess->start("python3", dmcPy);
//    qDebug() << QString("my init thread id:") << QThread::currentThreadId();
}

void DanmakuLauncher::launchDanmaku()
{
    while(!dmcPyProcess->atEnd())
    {
//        qDebug() << QString("my launch thread id:") << QThread::currentThreadId();
        QString newDanmaku(dmcPyProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(62, ' ');

        if(!dmp->isDanmakuVisible())
            return;

        int availDChannel = getAvailDanmakuChannel();
        //    if(checkVideoResolutionTimer == nullptr && (args.at(3) != "false"))
        //        danmakuRecorder->danmaku2ASS("", danmakuText, 13000, 24, availDChannel);
        int danmakuPos = availDChannel * (dmp->height() / 24);
        int danmakuSpeed = (dmp->width()+500) / 0.17;//0.17 pixel per second

        newDanmaku.remove(QRegExp("^\\[.*\\] "));
        emit sendDanmaku(newDanmaku, danmakuSpeed, danmakuPos);
        danmakuTimeNodeSeq[availDChannel] = time.elapsed();
        danmakuTimeLengthSeq[availDChannel] = (newDanmaku.length()*18 / 0.17) + 100;
    }
}

int DanmakuLauncher::getAvailDanmakuChannel()
{
    int currentTime = time.elapsed();
    int i;
    for(i = 0; i < 24; i++)
    {
        if((currentTime - danmakuTimeNodeSeq[i]) > danmakuTimeLengthSeq[i])
            return i;
    }
    i = qrand()%24;
    return i;
}
