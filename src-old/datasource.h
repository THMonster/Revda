#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QtCore>

#include "sites.h"
#include "room_model.h"

namespace DS {

struct Room
{
    QString url;
    QString cover;
    QString owner;
    QString title;
    bool status = false;
    bool like = false;
    bool his = false;
    int row = -1;
    int order_his = 0;
};

class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(RM::RoomModel *room_model, QObject *parent = nullptr);
    ~DataSource();

    void loadSettings();
    void saveSettings();

public slots:
    void addRoom(QString url, QString title, QString owner, QString cover, int status, bool open);
    void openQlphelper(QString url);
    void refresh();
    void openUrl(QString url);
    void toggleLike(int like, QString url);

private:
    QString urlToCode(QString url);
    void clear();
    void addHistory(QString code);

    RM::RoomModel *room_model = nullptr;
    QStandardItemModel *base_model = nullptr; // room, tag for which view, live status, order in history
    QHash<QString, DS::Room> rooms;
    Sites sites;

};

}
#endif // DATASOURCE_H
