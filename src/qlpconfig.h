#ifndef QLPCONFIG_H
#define QLPCONFIG_H

#include <QObject>

class QlpConfig : public QObject
{
    Q_OBJECT
public:
    static QlpConfig& getInstance()
    {
        static QlpConfig instance;
        return instance;
    }

    double readFontScale();
    double readFontAlpha();
    int readDanmakuSpeed();
    void writeFontScale(double fs);
    void writeFontAlpha(double fa);
    void writeDanmakuSpeed(int ms);

    uint fs_cnt = 0;
    uint fa_cnt = 0;
    uint sp_cnt = 0;

signals:

private:
    QlpConfig();
    ~QlpConfig();
};

#endif // QLPCONFIG_H
