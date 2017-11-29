#include "clirecorder.h"

CLIRecorder::CLIRecorder(QStringList args)
    : QObject()
{
    this->args = args;
    startStreamlinkProcess();
    mpvWidget = new MpvWidget(0, 0, true);
    mpvWidget->command(QStringList() << "loadfile" << namedPipe);

    QStringList dmcPy;
    dmcPy.append("-c");
    dmcPy.append("exec(\"\"\"\\nimport time, sys\\nimport threading\\nfrom danmu import DanMuClient\\ndef pp(msg):\\n    print(msg)\\n    sys.stdout.flush()\\ndmc = DanMuClient(sys.argv[1])\\nif not dmc.isValid(): \\n    print('Url not valid')\\n    sys.exit()\\n@dmc.danmu\\ndef danmu_fn(msg):\\n    pp('[%s] %s' % (msg['NickName'], msg['Content']))\\ndmc.start(blockThread = True)\\n\"\"\")");
    dmcPy.append(args.at(0));
    dmcPyProcess = new QProcess(this);
    connect(dmcPyProcess, &QProcess::readyReadStandardOutput, this, &CLIRecorder::readDanmaku);
    dmcPyProcess->start("python3", dmcPy);


    mTimer = new QTimer(this);
    mTimer->start(500);
    connect(mTimer, &QTimer::timeout, this, &CLIRecorder::checkVideoResolution);

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
//        danmakuPlayer->launchDanmaku(newDanmaku.remove(QRegExp("^\\[.*\\] ")));
        if(streamReady == true && (args.at(3) != "false"))
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
    if(mpvWidget->getProperty("video-params/w").toString() != QString("") && streamReady == false)
    {
        mTimer->stop();
        mTimer->deleteLater();
        streamReady = true;
        if(args.at(3) != "false")
//            danmakuRecorder = new DanmakuRecorder(getProperty("video-params/w").toInt(), getProperty("video-params/h").toInt(), QCoreApplication::arguments().at(3));
            danmakuRecorder = new DanmakuRecorder(1280, 720, args.at(3));
    }
//    else if(mpvWidget->getProperty("video-params/w").toString() == QString("") && streamReady == true)
//    {
//        mTimer->stop();
//        mTimer->deleteLater();
//        QApplication::exit(2);
//    }
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
    namedPipe = "/tmp/qlivesplayersdfadsfsdewe";
    QProcess::execute("mkfifo " + namedPipe);
    streamlinkProcess = new QProcess(this);
    if(args.at(2) == QString("false")) {
        streamlinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O > " + namedPipe + "\"");
    } else
        streamlinkProcess->start("bash -c \"streamlink " + args.at(0) + " " + args.at(1) + " -O | tee " + args.at(2) + " > " + namedPipe + "\"");
    if(!streamlinkProcess->waitForStarted())
        QApplication::exit(1);

//    connect(streamlinkProcess, &QProcess::finished, this, &CLIRecorder::onStreamlinkFinished);
    connect(streamlinkProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CLIRecorder::onStreamlinkFinished);
}

void CLIRecorder::onStreamlinkFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Now Quit";
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    QApplication::exit(0);
}
