#ifndef MainWindow_H
#define MainWindow_H

#include <QtGui>
#include <QMainWindow>
#include <QWidget>
#include <QQuickWidget>
#include <QShortcut>

#include "room_model.h"
#include "datasource.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


signals:

public slots:

private:
    QQuickWidget* root_qml = nullptr;
    RM::RoomModel *room_model = nullptr;
    DS::DataSource *data_source = nullptr;
};

#endif // MAINWINDOW_H
