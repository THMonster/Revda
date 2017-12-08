#include "danmakulauncher.h"

DanmakuLauncher::DanmakuLauncher(QStringList args, QWidget *parent)
        :QObject(0)
{
    this->args = args;
    dglw = parent;


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
//        QMutexLocker lock(&mutex);
        mutex.lock();
        if(!danmakuQueue.isEmpty())
        {
            if ((*danmakuQueue.begin()).posX + (*danmakuQueue.begin()).length < 0)
            {
                danmakuQueue.dequeue();
                if ((*danmakuQueue.begin()).posX + (*danmakuQueue.begin()).length < 0)
                {
                    danmakuQueue.dequeue();
                }
            }
        }
        mutex.unlock();

//        qDebug() << QString("my launch thread id:") << QThread::currentThreadId();
        QString newDanmaku(dmcPyProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(62, ' ');

        if(!danmakuShowFlag)
            return;

        Danmaku_t d;

        newDanmaku.remove(QRegExp("^\\[.*\\] "));
        d.text = newDanmaku;
//        qDebug() << dglw->width();
        d.posX = dglw->width();
        QFontMetrics fm(font);
        d.length = fm.width(newDanmaku);
        d.step = 2.0 * sqrt(sqrt(d.length/250.0)) + 0.5;
//        qDebug() << d.step;
        int availDChannel = getAvailDanmakuChannel(d.step);
        //    if(checkVideoResolutionTimer == nullptr && (args.at(3) != "false"))
        //        danmakuRecorder->danmaku2ASS("", danmakuText, 13000, 24, availDChannel);
        int danmakuPos = availDChannel * (dglw->height() / 24);
        d.posY = danmakuPos;
        mutex.lock();
        danmakuQueue.enqueue(d);
        mutex.unlock();

        danmakuTimeNodeSeq[availDChannel] = time.elapsed();
        danmakuWidthSeq[availDChannel] = d.length;
        danmakuSpeedSeq[availDChannel] = d.step;
    }
}

int DanmakuLauncher::getAvailDanmakuChannel(double currentSpeed)
{
    int currentTime = time.elapsed();
    int i;
    for(i = 0; i < 24; i++)
    {
        if ((((currentTime - danmakuTimeNodeSeq[i]) * 60 * currentSpeed / 1000) - danmakuWidthSeq[i]) > 0)
        {
            if ((((double)(currentTime - danmakuTimeNodeSeq[i]) * danmakuSpeedSeq[i] / 16.67) - danmakuWidthSeq[i]) / (currentSpeed - danmakuSpeedSeq[i]) > ((double)dglw->width() / currentSpeed))
            {
                return i;
            }
            else if (currentSpeed - danmakuSpeedSeq[i] <= 0)
            {
                return i;
            }
        }
    }
    i = qrand()%24;
    return i;
}

void DanmakuLauncher::paintDanmaku(QPainter *painter)
{
    QMutexLocker lock(&mutex);

    painter->setFont(font);

    QQueue<Danmaku_t>::iterator i;
    for (i = danmakuQueue.begin(); i != danmakuQueue.end(); ++i)
    {
        painter->setPen(borderPen);
        painter->drawText(QPointF(i->posX, i->posY+20.0), i->text);
        painter->setPen(textPen);
        painter->drawText(QPointF(i->posX-1.0, i->posY+19.0), i->text);
        (*i).posX = (*i).posX - (*i).step;
    }
}

void DanmakuLauncher::initDL()
{
    borderPen = QPen(Qt::black);

    textPen.setColor(Qt::white);

    font.setPixelSize(20);
    font.setBold(true);

    for (int i = 0; i < 24; i++)
    {
        danmakuWidthSeq[i] = -1000000;
        danmakuTimeNodeSeq[i] = -100000;
    }
    initDmcPy();
}

void DanmakuLauncher::clearDanmakuQueue()
{
    QMutexLocker lock(&mutex);
    danmakuQueue.clear();
    danmakuShowFlag = false;
}

void DanmakuLauncher::setDanmakuShowFlag(bool flag)
{
    danmakuShowFlag = flag;
}


