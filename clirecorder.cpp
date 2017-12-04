#include "clirecorder.h"

CLIRecorder::CLIRecorder(QStringList args)
    : QObject()
{
    this->args = args;

    qDebug() << "Waiting for stream...";
    checkProcess = new QProcess(this);
    checkProcess->start("bash -c \"streamlink " + args.at(0) + "\"");
    connect(checkProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CLIRecorder::checkStreamAvailable);

    mpvWidget = new MpvWidget(0, 0, true);

    QStringList dmcPy;
    dmcPy.append("-c");
    dmcPy.append("exec(\"\"\"\\nimport time, sys\\nimport threading\\nfrom danmu import DanMuClient\\ndef pp(msg):\\n    print(msg)\\n    sys.stdout.flush()\\ndmc = DanMuClient(sys.argv[1])\\nif not dmc.isValid(): \\n    print('Url not valid')\\n    sys.exit()\\n@dmc.danmu\\ndef danmu_fn(msg):\\n    pp('[%s] %s' % (msg['NickName'], msg['Content']))\\ndmc.start(blockThread = True)\\n\"\"\")");
    dmcPy.append(args.at(0));
    dmcPyProcess = new QProcess(this);
    connect(dmcPyProcess, &QProcess::readyReadStandardOutput, this, &CLIRecorder::readDanmaku);
    dmcPyProcess->start("python3", dmcPy);


    checkStreamReadyTimer = new QTimer(this);
    connect(checkStreamReadyTimer, &QTimer::timeout, this, &CLIRecorder::checkVideoResolution);

    time.start();
}

CLIRecorder::~CLIRecorder()
{
    QProcess::execute("rm " + namedPipe);
    delete danmakuRecorder;
    dmcPyProcess->terminate();
    dmcPyProcess->waitForFinished(3000);
    dmcPyProcess->deleteLater();
    streamlinkProcess->terminate();
    streamlinkProcess->waitForFinished(3000);
    streamlinkProcess->deleteLater();
}

void CLIRecorder::readDanmaku()
{
    while(!dmcPyProcess->atEnd())
    {
        QString newDanmaku(dmcPyProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(62, ' ');
        QRegExp re("^\\[(.*)\\] ");
        re.indexIn(newDanmaku);
//        qDebug().noquote() << re.cap(1);
        if(streamReady == true && (args.at(3) != "false"))
        {
            int availDChannel = getAvailDanmakuChannel();
            danmakuRecorder->danmaku2ASS(re.cap(1), newDanmaku.remove(QRegExp("^\\[.*\\] ")), 13000, 24, availDChannel);
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
        checkStreamReadyTimer->stop();
        if(args.at(3) != "false" && danmakuRecorder == nullptr)
//            danmakuRecorder = new DanmakuRecorder(getProperty("video-params/w").toInt(), getProperty("video-params/h").toInt(), QCoreApplication::arguments().at(3));
            danmakuRecorder = new DanmakuRecorder(1280, 720, args.at(3));
        streamReady = true;
        danmakuRecorder->resume();
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

void CLIRecorder::startStreamlinkProcess()
{
    if(namedPipe == QString(""))
    {
        namedPipe = "/tmp/qlivesplayer-" + QUuid::createUuid().toString();
        QProcess::execute("mkfifo " + namedPipe);
        streamlinkProcess = new QProcess(this);
        connect(streamlinkProcess, &QProcess::readyReadStandardError, this, &CLIRecorder::onStreamlinkStderrReady);
        connect(streamlinkProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CLIRecorder::onStreamlinkFinished);
    }
    if(streamAvailable)
    {

        if(args.at(2) == QString("false")) {
            streamlinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O > " + namedPipe + "\"");
        } else
            streamlinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O | tee " + args.at(2) + QString(".%1").arg(videoPart++) + " > " + namedPipe + "\"");
        if(!streamlinkProcess->waitForStarted())
            QApplication::exit(1);
        mpvWidget->command(QStringList() << "loadfile" << namedPipe);
    }

}

void CLIRecorder::onStreamlinkFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    streamAvailable = false;
    streamReady = false;
    danmakuRecorder->pause();
    mpvWidget->command(QStringList() << "stop");
    qDebug().noquote() << "Streamlink quited, now try to restart...";
    checkProcess->start("bash -c \"streamlink " + args.at(0) + "\"");

}

void CLIRecorder::checkStreamAvailable()
{
    QThread::msleep(500);
    QString stdinfo(checkProcess->readAllStandardOutput());
//    qDebug() << stdinfo;
    if(stdinfo.contains(QRegExp("Available streams")))
    {
        streamAvailable = true;
        QThread::msleep(500);
        startStreamlinkProcess();
        checkStreamReadyTimer->start(100);    
    }
    else
    {
        streamAvailable = false;
        checkProcess->start("bash -c \"streamlink " + args.at(0) + "\"");
    }
}

void CLIRecorder::onStreamlinkStderrReady()
{
    QString stdinfo(streamlinkProcess->readAllStandardError());
    stdinfo.chop(1);
    qDebug().noquote() << stdinfo;

}
