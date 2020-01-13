#include <QVBoxLayout>
#include <QQmlContext>
#include <QQuickItem>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->resize(QSize(1080, 660));
    sc_quit = new QShortcut(QKeySequence::Quit, this);
    connect(sc_quit, &QShortcut::activated, [=](){
        QCoreApplication::exit(0);
    });


    room_model = new RoomModel(this);

    root_qml = new QQuickWidget(this);
    root_qml->rootContext()->setContextProperty("room_model", room_model);
    root_qml->setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    root_qml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    QQuickItem* header = root_qml->rootObject()->findChild<QQuickItem *>(QStringLiteral("header"));
//    QQuickItem* gv = root_qml->rootObject()->findChild<QQuickItem *>(QStringLiteral("grid_view"));
    QList<QQuickItem *> gv = root_qml->rootObject()->findChildren<QQuickItem *>(QStringLiteral("grid_view"));
    QObject::connect(gv[0], SIGNAL(room_clicked(QString)), room_model, SLOT(openRoom(QString)));
    QObject::connect(gv[1], SIGNAL(room_clicked(QString)), room_model, SLOT(openRoom(QString)));
    QObject::connect(gv[0], SIGNAL(like_clicked(int, QString)), room_model, SLOT(toggleLike(int, QString)));
    QObject::connect(gv[1], SIGNAL(like_clicked(int, QString)), room_model, SLOT(toggleLike(int, QString)));
//    QObject::connect(gv[1], SIGNAL(cu_clicked(QString)), this, SLOT(apply_color(QString)));
    QObject::connect(header, SIGNAL(refresh_clicked()), room_model, SLOT(refresh()));
    QObject::connect(header, SIGNAL(open_clicked(QString)), room_model, SLOT(openUrl(QString)));

    setCentralWidget(root_qml);

    QShortcut *sc_refresh = new QShortcut(QKeySequence::Refresh, this);
    connect(sc_refresh, &QShortcut::activated, room_model, &RoomModel::refresh);
    room_model->load();
    room_model->refresh();
}

MainWindow::~MainWindow()
{

}
