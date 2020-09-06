#ifndef ROOM_MODEL_H
#define ROOM_MODEL_H

#include <QObject>
#include <QtGui>
#include "sites.h"

class Room
{
    Q_GADGET
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString cover READ cover)
    Q_PROPERTY(QString owner READ owner)
    Q_PROPERTY(int status READ status)
    Q_PROPERTY(QString url READ url)
    Q_PROPERTY(int like READ like)

public:
    Room(QString url, QString title, QString cover, QString owner, int status, int like) {
        m_url = url;
        m_cover = cover;
        m_title = title;
        m_owner = owner;
        m_status = status;
        m_like = like;
    }
    Room() = default;
    Room(const Room& other)=default;
    Room& operator=(const Room& other)=default;

    const QString& cover() {
        return m_cover;
    }
    const QString& title() const {
        return m_title;
    }
    const QString& url() const {
        return m_url;
    }
    const QString& owner() const {
        return m_owner;
    }
    const int& status() const {
        return m_status;
    }
    const int& like() const {
        return m_like;
    }


private:
    QString m_title;
    QString m_cover;
    QString m_owner;
    QString m_url;
    int m_status = 0;
    int m_like = 0;
};

class RoomModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStandardItemModel* saved_model READ saved_model)
    Q_PROPERTY(QStandardItemModel* history_model READ history_model)
    Q_DISABLE_COPY(RoomModel)

public:
    explicit RoomModel(QObject *parent = nullptr);
    ~RoomModel();
    QStandardItemModel* saved_model() const;
    QStandardItemModel* history_model() const;
    void load();
    void addRoomToModel(int cata, QString url, QString title, QString owner, QString cover, int status, int num);
    void save();
    QString urlToCode(QString url);
    void addUrlToHistory(QString url);

public slots:
    void openRoom(QString url);
    void openUnverifiedRoom(QString url);
    void refresh(bool history_only = false);
//    void refreshHistory();
    void openUrl(QString url);
    void toggleLike(int like, QString url);



private:
    QStandardItemModel* m_saved_model = nullptr;
    QStandardItemModel* m_history_model = nullptr;
    QStringList saved;
    QStringList history;
    QSettings* settings;
    Sites sites;

};

#endif // ROOM_MODEL_H
