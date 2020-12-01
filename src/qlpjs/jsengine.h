#ifndef JSENGINE_H
#define JSENGINE_H

#include <QObject>
#include <QtCore>
#include <QJSEngine>

class JSConsole : public QObject
{
    Q_OBJECT
public:
    explicit JSConsole(QObject *parent = 0)
        : QObject(parent)
    {

    }

signals:

public slots:
    void log(QString msg)
    {
        QTextStream(stdout) << msg << '\n';
    }
};

class JSEngine : public QObject
{
    Q_OBJECT
public:
    explicit JSEngine(QObject *parent = nullptr);

    void run();

private:
    QJSEngine *jse = nullptr;
};

#endif // JSENGINE_H
