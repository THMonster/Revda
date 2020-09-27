#include "qlpcmdparser.h"

QlpCmdParser::QlpCmdParser(QString s)
{
//    qInfo() << s;
    bool ok = false;
    auto t = s.splitRef(QStringLiteral("qlp:"), QString::SkipEmptyParts);
    auto args = t.last().split(',', QString::SkipEmptyParts);
    for (const auto& arg : args) {
        auto t2 = arg.split('=', QString::SkipEmptyParts);
        if ((t2.length() >= 2) && ((t2[0].compare(QStringLiteral("q")) == 0) || (t2[0].compare(QStringLiteral("quality")) == 0))) {
            quality = t2[1].toInt(&ok);
            if (ok == false) {
                quality = -1;
            }
        } else if ((t2[0].compare(QStringLiteral("r")) == 0) || (t2[0].compare(QStringLiteral("reload")) == 0)) {
            reload = true;
        } else if ((t2.length() >= 2) && (t2[0].compare(QStringLiteral("fs")) == 0)) {
            fs = t2[1].toDouble(&ok);
            if (ok == false) {
                fs = -1;
            }
        } else if ((t2.length() >= 2) && (t2[0].compare(QStringLiteral("fa"))) == 0) {
            fa = t2[1].toDouble(&ok);
            if (ok == false) {
                fa = -1;
            }
        } else if ((t2.length() >= 2) && ((t2[0].compare(QStringLiteral("p")) == 0) || (t2[0].compare(QStringLiteral("page")) == 0))) {
            page = t2[1].toInt(&ok);
            if (ok == false) {
                page = -1;
            }
        } else if ((t2[0].compare(QStringLiteral("next")) == 0)) {
            page_next = true;
        } else if ((t2[0].compare(QStringLiteral("back")) == 0)) {
            page_back = true;
        }
    }
}

QlpCmdParser::~QlpCmdParser()
{

}

bool QlpCmdParser::getReload()
{
    return reload;
}

int QlpCmdParser::getQuality()
{
    return quality;
}

double QlpCmdParser::getFs()
{
    return fs;
}

double QlpCmdParser::getFa()
{
    return fa;
}

bool QlpCmdParser::getPageNext()
{
    return page_next;
}

bool QlpCmdParser::getPageBack()
{
    return page_back;
}

int QlpCmdParser::getPage()
{
    return page;
}
