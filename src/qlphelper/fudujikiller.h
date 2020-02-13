#ifndef FUDUJIKILLER_H
#define FUDUJIKILLER_H
#include <QtCore>

class FudujiKiller
{
public:
    FudujiKiller();
    void addTime();
    bool checkDanmaku(const QString& d);
    void ebb();

private:
    quint64 time = 0;
    QHash<QString, int> danmaku_stats;
};

#endif // FUDUJIKILLER_H
