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
        QMutexLocker lock(&mutex);
        if(!danmakuQueue.isEmpty())
        {
            if ((*danmakuQueue.begin()).posX < -500)
            {
                danmakuQueue.dequeue();
                if ((*danmakuQueue.begin()).posX < -500)
                {
                    danmakuQueue.dequeue();
                }
            }
        }

//        qDebug() << QString("my launch thread id:") << QThread::currentThreadId();
        QString newDanmaku(dmcPyProcess->readLine());
        qDebug().noquote() << newDanmaku.remove(QRegExp("\n$")).leftJustified(62, ' ');

//        if(!dglw->isDanmakuVisible())
//            return;

        Danmaku_t d;

        int availDChannel = getAvailDanmakuChannel();
        //    if(checkVideoResolutionTimer == nullptr && (args.at(3) != "false"))
        //        danmakuRecorder->danmaku2ASS("", danmakuText, 13000, 24, availDChannel);

        int danmakuPos = availDChannel * (dglw->height() / 24);
//        int danmakuSpeed = (dmp->width()+500) / 0.17;//0.17 pixel per second

        newDanmaku.remove(QRegExp("^\\[.*\\] "));
        d.text = newDanmaku;
        d.posY = danmakuPos;
//        qDebug() << dglw->width();
        d.posX = dglw->width();
        d.step = 3;
        danmakuQueue.enqueue(d);
//        QFontMetrics fm(font);
//        emit sendDanmaku(newDanmaku, danmakuSpeed, danmakuPos);
        danmakuTimeNodeSeq[availDChannel] = time.elapsed();
        danmakuTimeLengthSeq[availDChannel] = (500 / 0.15) + 100;
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

void DanmakuLauncher::paintDanmaku(QPainter *painter)
{
//    qDebug() << "hello";
    QMutexLocker lock(&mutex);
//    QPainterPath textPath;

//    painter->setPen(borderPen);
//    painter->setBrush(textBrush);

    painter->setFont(font);
//    painter->setCompositionMode (QPainter::CompositionMode_Source);
//    painter->fillRect(event->rect(), Qt::transparent);
//    painter->setCompositionMode (QPainter::CompositionMode_SourceOver);

    QQueue<Danmaku_t>::iterator i;
    for (i = danmakuQueue.begin(); i != danmakuQueue.end(); ++i)
    {
//        textPath.addText(i->posX, i->posY+20, font, i->text);
        painter->setPen(borderPen);
        painter->drawText(i->posX, i->posY+20, i->text);
        painter->setPen(textPen);
        painter->drawText(i->posX-1, i->posY+19, i->text);
        (*i).posX = (*i).posX - (*i).step;
    }
//    painter->drawPath(textPath);
}

void DanmakuLauncher::initDL()
{
    borderPen = QPen(Qt::black);
//    borderPen.setWidth(1);
    textPen.setColor(Qt::white);
//    textBrush.setColor(Qt::white);
//    textBrush.setStyle(Qt::SolidPattern);
//    font.setFamily("Source Han Sans CN");
    font.setPixelSize(20);
    font.setBold(true);
//    font.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
//    paintTimer = new QTimer(this);
//    paintTimer->start(16);
//    connect(paintTimer, &QTimer::timeout, dglw, &DanmakuGLWidget::animate);
    initDmcPy();
}
