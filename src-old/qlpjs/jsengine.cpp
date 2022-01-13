#include "jsengine.h"

#include <iostream>

JSEngine::JSEngine(QObject *parent) : QObject(parent)
{
    jse = new QJSEngine(this);
    auto console = new JSConsole(this);
    jse->globalObject().setProperty("console", jse->newQObject(console));
}

void JSEngine::run()
{
    QString js;
    std::string line;
    while (!std::cin.eof()) {
        std::getline(std::cin, line);
        if (line.length() == 0) {
            continue;
        }
        js.append(QString::fromStdString(line) + '\n');
    }
    //        QTextStream(stderr) << "JS> ";
    auto result = jse->evaluate(js);
    if (result.isError()) {
        QTextStream(stdout) << "Uncaught exception at line"
                            << result.property("lineNumber").toInt()
                            << ":" << result.toString() << '\n';

    } else {
        QTextStream(stdout) << result.toString() << '\n';
    }
}
