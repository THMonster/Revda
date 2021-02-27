#include <QtCore>

#include "qlpconfig.h"

double QlpConfig::readFontScale()
{
    QSettings s("QLivePlayer", "QLivePlayer");
    auto ret = s.value(QStringLiteral("font_scale"), 1.0).toDouble();
    if (ret > 0) {
        return ret;
    } else {
        return 1.0;
    }
}

double QlpConfig::readFontAlpha()
{
    QSettings s("QLivePlayer", "QLivePlayer");
    auto ret = s.value(QStringLiteral("font_alpha"), 0.0).toDouble();
    if (ret >= 0 && ret <= 1) {
        return ret;
    } else {
        return 0.0;
    }
}

int QlpConfig::readDanmakuSpeed()
{
    QSettings s("QLivePlayer", "QLivePlayer");
    auto ret = s.value(QStringLiteral("danmaku_speed"), 8000).toInt();
    if (ret > 500 && ret < 30000) {
        return ret;
    } else {
        return 8000;
    }
}

void QlpConfig::writeFontScale(double fs)
{
    ++fs_cnt;
    QTimer::singleShot(1000, [this, fs]() {
        if (this->fs_cnt <= 1) {
            QSettings s("QLivePlayer", "QLivePlayer");
            s.setValue("font_scale", fs);
        }
        --fs_cnt;
    });
}

void QlpConfig::writeFontAlpha(double fa)
{
    ++fa_cnt;
    QTimer::singleShot(1000, [this, fa]() {
        if (this->fa_cnt <= 1) {
            QSettings s("QLivePlayer", "QLivePlayer");
            s.setValue("font_alpha", fa);
        }
        --fa_cnt;
    });
}

void QlpConfig::writeDanmakuSpeed(int ms)
{
    ++sp_cnt;
    QTimer::singleShot(1000, [this, ms]() {
        if (this->sp_cnt <= 1) {
            QSettings s("QLivePlayer", "QLivePlayer");
            s.setValue("danmaku_speed", ms);
        }
        --sp_cnt;
    });
}

QlpConfig::QlpConfig()
{

}

QlpConfig::~QlpConfig()
{

}
