#include <QVBoxLayout>
#include <QQmlContext>
#include <QQuickItem>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->resize(QSize(1080, 660));
    QShortcut *sc_quit = new QShortcut(QKeySequence::Quit, this);
    connect(sc_quit, &QShortcut::activated, [=](){
        QCoreApplication::exit(0);
    });


    room_model = new RM::RoomModel(this);
    data_source = new DS::DataSource(room_model, this);


    root_qml = new QQuickWidget(this);
    root_qml->rootContext()->setContextProperty("room_model", room_model);
    root_qml->setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    root_qml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    QQuickItem* header = root_qml->rootObject()->findChild<QQuickItem *>(QStringLiteral("header"));
    QList<QQuickItem *> gv = root_qml->rootObject()->findChildren<QQuickItem *>(QStringLiteral("grid_view"));
    QObject::connect(gv[0], SIGNAL(room_clicked(QString)), data_source, SLOT(openQlphelper(QString)));
    QObject::connect(gv[1], SIGNAL(room_clicked(QString)), data_source, SLOT(openQlphelper(QString)));
    QObject::connect(gv[0], SIGNAL(like_clicked(int, QString)), data_source, SLOT(toggleLike(int, QString)));
    QObject::connect(gv[1], SIGNAL(like_clicked(int, QString)), data_source, SLOT(toggleLike(int, QString)));
    QObject::connect(header, SIGNAL(refresh_clicked()), data_source, SLOT(refresh()));
    QObject::connect(header, SIGNAL(open_clicked(QString)), data_source, SLOT(openUrl(QString)));

    setCentralWidget(root_qml);

    auto sc_refresh = new QShortcut(QKeySequence::Refresh, this);
    auto sc_refresh1 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this);
//    connect(sc_refresh, &QShortcut::activated, room_model, &RoomModel::refresh);
    connect(sc_refresh, &QShortcut::activated, [this]() {
        this->data_source->refresh();
    });
    connect(sc_refresh1, &QShortcut::activated, [this]() {
        this->data_source->refresh();
    });
    data_source->loadSettings();
    data_source->refresh();
}

MainWindow::~MainWindow()
{

}
