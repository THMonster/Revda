#include "danmakulauncher.h"

DanmakuLauncher::DanmakuLauncher(QStringList args, QWidget *parent, int resWidth, int resHeight)
        :QObject(0)
{
    this->args = args;
    dglw = parent;
    this->resWidth = resWidth;
    this->resHeight = resHeight;
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

void DanmakuLauncher::initDRecorder()
{
    if (args.at(3) != "false") {
        danmakuRecorder = new DanmakuRecorder(1280, 720, args.at(3));
    } else {
        danmakuRecorder = nullptr;
    }
}

void DanmakuLauncher::launchDanmaku()
{
    while(!dmcPyProcess->atEnd())
    {
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
        updateResolution();
        mutex.unlock();
        QString newDanmaku(dmcPyProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(62, ' ');

        if(!danmakuShowFlag)
            return;

        Danmaku_t d;

        QRegExp re("^\\[(.*)\\] ");
        re.indexIn(newDanmaku);
        d.speaker = re.cap(1);
        d.text = newDanmaku.remove(QRegExp("^\\[.*\\] "));;
//        qDebug() << dglw->width();
        d.posX = resWidth;
        QFontMetrics fm(font);
        d.length = fm.width(newDanmaku);
        d.step = 2.0 * sqrt(sqrt(d.length/250.0)) + 0.5;
//        qDebug() << d.step;

        int availDChannel = getAvailDanmakuChannel(d.step);
        if (availDChannel < 0 && danmakuRecorder == nullptr)
            return;
        int danmakuPos = availDChannel * (resHeight / 24);
        d.posY = danmakuPos;

        if (danmakuRecorder != nullptr && streamReady == true) {
            danmakuRecorder->start();
            int duration = ((double)resWidth + d.length) / (60.0 * d.step) * 1000.0;
            danmakuRecorder->danmaku2ASS(d.speaker, d.text, duration, d.length, 24, availDChannel);
        }

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
            if ((((double)(currentTime - danmakuTimeNodeSeq[i]) * danmakuSpeedSeq[i] / 16.67) - danmakuWidthSeq[i]) / (currentSpeed - danmakuSpeedSeq[i]) > ((double)resWidth / currentSpeed))
            {
                return i;
            }
            else if (currentSpeed - danmakuSpeedSeq[i] <= 0)
            {
                return i;
            }
        }
    }
//    i = qrand()%24;
    return -4;
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

void DanmakuLauncher::paintDanmakuCLI()
{
    QMutexLocker lock(&mutex);

    QQueue<Danmaku_t>::iterator i;
    for (i = danmakuQueue.begin(); i != danmakuQueue.end(); ++i)
    {
        (*i).posX = (*i).posX - (*i).step;
    }
}

void DanmakuLauncher::initDL()
{
    borderPen = QPen(Qt::black);

    textPen.setColor(Qt::white);

    font.setPixelSize(20);
//    font.setBold(true);

    for (int i = 0; i < 24; i++)
    {
        danmakuWidthSeq[i] = -1000000;
        danmakuTimeNodeSeq[i] = -100000;
    }
    initDmcPy();
    initDRecorder();
}

void DanmakuLauncher::clearDanmakuQueue()
{
    QMutexLocker lock(&mutex);
    danmakuQueue.clear();
}

void DanmakuLauncher::setDanmakuShowFlag(bool flag)
{
    QMutexLocker lock(&mutex);
    danmakuShowFlag = flag;
}

void DanmakuLauncher::setStreamReadyFlag(bool flag)
{
    QMutexLocker lock(&mutex);
    streamReady = flag;
}

void DanmakuLauncher::updateResolution()
{
    if (dglw != nullptr) {
        resWidth = dglw->width();
        resHeight = dglw->height();
    }
}

void DanmakuLauncher::setPlayingState(int state)
{
    if (state == 0) {
        danmakuRecorder->resume();
    } else if (state == 1) {
        danmakuRecorder->pause();
    }
}


