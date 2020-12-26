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

QlpConfig::QlpConfig()
{

}

QlpConfig::~QlpConfig()
{

}
