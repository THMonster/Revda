#ifndef MPVEMWIDGET_H
#define MPVEMWIDGET_H

#include <QtGui>
#include <mpv/client.h>
#include <mpv/qthelper.hpp>
#include <QWidget>


class MpvEmWidget : public QWidget
{
    Q_OBJECT

signals:
    void mpv_events();

private:
    mpv_handle *mpv;

    void create_player();
    void handle_mpv_event(mpv_event *event);

private slots:
    void on_mpv_events();

public:
    MpvEmWidget(QWidget *parent = 0, Qt::WindowFlags f = 0, bool cli = false);
    void openFile(QString filePath);
    ~MpvEmWidget();
    QVariant getProperty(const QString &name) const;
    void command(const QVariant &params);
};

#endif // MPVEMWIDGET_H
