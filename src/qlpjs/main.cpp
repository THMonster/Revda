#include <QCoreApplication>
#include <QtCore>

#include "jsengine.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JSEngine jse;
    jse.run();
//    return a.exec();
}
