#include "room_model.h"

using namespace RM;

RoomModel::RoomModel(QObject *parent) : QObject(parent)
{
    base_model = new QStandardItemModel(this);
    m_history_model = new QSortFilterProxyModel(this);
    m_saved_model = new QSortFilterProxyModel(this);
    m_saved_model->setSourceModel(base_model);
    m_history_model->setSourceModel(base_model);

    m_saved_model->setFilterFixedString("S");
    m_saved_model->setFilterKeyColumn(1);
    m_history_model->setFilterFixedString("H");
    m_history_model->setFilterKeyColumn(1);
}

RoomModel::~RoomModel()
{
    //    save();
}

void RoomModel::sort()
{
    m_saved_model->sort(2, Qt::DescendingOrder);
    m_history_model->sort(3, Qt::AscendingOrder);
}

QSortFilterProxyModel *RoomModel::saved_model() const
{
    return m_saved_model;
}

QSortFilterProxyModel *RoomModel::history_model() const
{
    return m_history_model;
}

