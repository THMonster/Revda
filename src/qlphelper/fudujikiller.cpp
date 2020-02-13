#include "fudujikiller.h"

FudujiKiller::FudujiKiller()
{

}

void FudujiKiller::addTime()
{
    ++time;
}

bool FudujiKiller::checkDanmaku(const QString &d)
{
    bool ret = true;
    QHash<QString, int>::iterator i = danmaku_stats.find(d);
    if (i != danmaku_stats.end()) {
        i.value() = i.value() + 1;
        if (i.value() > 40) {
            if (rand() % 20 != 0) {
                ret = false;
            }
        }
    } else {
        danmaku_stats[d] = 1;
    }

    return ret;
}

void FudujiKiller::ebb()
{
    if (time % 10 == 0) {
        QHash<QString, int>::iterator i = danmaku_stats.begin();

        while (i != danmaku_stats.end()) {
//            qInfo() << i.value();
            if (i.value() <= 10) {
                i = danmaku_stats.erase(i);
            } else {
                i.value() = i.value() - 10;
                ++i;
            }
        }
    }
}
