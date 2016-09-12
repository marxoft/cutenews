/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "transfermodel.h"
#include "transfers.h"
#ifdef WIDGETS_UI
#include "utils.h"
#include <QColor>
#endif

TransferModel::TransferModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_roles[BytesTransferredRole] = "bytesTransferred";
    m_roles[CategoryRole] = "category";
    m_roles[CustomCommandRole] = "customCommand";
    m_roles[CustomCommandOverrideEnabledRole] = "customCommandOverrideEnabled";
    m_roles[DownloadPathRole] = "downloadPath";
    m_roles[ErrorStringRole] = "errorString";
    m_roles[FileNameRole] = "fileName";
    m_roles[IdRole] = "id";
    m_roles[NameRole] = "name";
    m_roles[PriorityRole] = "priority";
    m_roles[PriorityStringRole] = "priorityString";
    m_roles[ProgressRole] = "progress";
    m_roles[ProgressStringRole] = "progressString";
    m_roles[SizeRole] = "size";
    m_roles[StatusRole] = "status";
    m_roles[StatusStringRole] = "statusString";
    m_roles[SubscriptionIdRole] = "subscriptionId";
    m_roles[TransferTypeRole] = "transferType";
    m_roles[UrlRole] = "url";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif    
    for (int i = 0; i < Transfers::instance()->count(); i++) {
        if (Transfer *transfer = Transfers::instance()->get(i)) {
            onTransferAdded(transfer);
        }
    }
    
    connect(Transfers::instance(), SIGNAL(countChanged(int)), this, SLOT(onCountChanged(int)));
    connect(Transfers::instance(), SIGNAL(transferAdded(Transfer*)), this, SLOT(onTransferAdded(Transfer*)));
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> TransferModel::roleNames() const {
    return m_roles;
}
#endif

int TransferModel::rowCount(const QModelIndex &) const {
    return Transfers::instance()->count();
}

#ifdef WIDGETS_UI
int TransferModel::columnCount(const QModelIndex &) const {
    return 5;
}

QVariant TransferModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Category");
        case 2:
            return tr("Priority");
        case 3:
            return tr("Progress");
        case 4:
            return tr("Status");
        default:
            break;
        }
    }
    
    return QVariant();
}
#endif

QVariant TransferModel::data(const QModelIndex &index, int role) const {
    if (const Transfer *transfer = Transfers::instance()->get(index.row())) {
#ifdef WIDGETS_UI
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0:
                return transfer->property("name");
            case 1:
                return transfer->property("category");
            case 2:
                return transfer->property("priorityString");
            case 3:
                return transfer->property("progressString");
            case 4:
                return transfer->property("statusString");
            default:
                return QVariant();
            }
        }
#endif
        return transfer->property(m_roles[role]);
    }
    
    return QVariant();
}

QMap<int, QVariant> TransferModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    
    if (const Transfer *transfer = Transfers::instance()->get(index.row())) {
        QHashIterator<int, QByteArray> iterator(m_roles);
        
        while (iterator.hasNext()) {
            iterator.next();
            map[iterator.key()] = transfer->property(iterator.value());
        }
    }
    
    return map;
}

bool TransferModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (Transfer *transfer = Transfers::instance()->get(index.row())) {
        return transfer->setProperty(m_roles[role], value);
    }
    
    return false;
}

bool TransferModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    QMapIterator<int, QVariant> iterator(roles);
    
    while (iterator.hasNext()) {
        iterator.next();
        
        if (!setData(index, iterator.value(), iterator.key())) {
            return false;
        }
    }
    
    return true;
}

QVariant TransferModel::data(int row, const QByteArray &role) const {
    if (const Transfer *transfer = Transfers::instance()->get(row)) {
        return transfer->property(role);
    }

    return QVariant();
}

QVariantMap TransferModel::itemData(int row) const {
    QVariantMap map;
    
    if (const Transfer *transfer = Transfers::instance()->get(row)) {
        foreach (const QByteArray &value, m_roles.values()) {
            map[value] = transfer->property(value);
        }
    }
    
    return map;
}

bool TransferModel::setData(int row, const QVariant &value, const QByteArray &role) {
    if (Transfer *transfer = Transfers::instance()->get(row)) {
        return transfer->setProperty(role, value);
    }

    return false;
}

bool TransferModel::setItemData(int row, const QVariantMap &roles) {
    QMapIterator<QString, QVariant> iterator(roles);
    
    while (iterator.hasNext()) {
        iterator.next();
        
        if (!setData(row, iterator.value(), iterator.key().toUtf8())) {
            return false;
        }
    }
    
    return true;
}
QModelIndexList TransferModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                     Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int TransferModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList indexes = match(index(start), m_roles.key(role), value, Qt::MatchFlags(flags));
    return indexes.isEmpty() ? -1 : indexes.first().row();
}

int TransferModel::indexOf(Transfer *transfer) const {
    for (int i = 0; i < Transfers::instance()->count(); i++) {
        if (Transfers::instance()->get(i) == transfer) {
            return i;
        }
    }
    
    return -1;
}

void TransferModel::onCountChanged(int count) {
    beginResetModel();
    endResetModel();
    emit countChanged(count);
}

void TransferModel::onTransferAdded(Transfer *transfer) {
#ifdef WIDGETS_UI
    connect(transfer, SIGNAL(nameChanged()), this, SLOT(onTransferNameChanged()));
    connect(transfer, SIGNAL(categoryChanged()), this, SLOT(onTransferCategoryChanged()));
    connect(transfer, SIGNAL(priorityChanged()), this, SLOT(onTransferPriorityChanged()));
    connect(transfer, SIGNAL(progressChanged()), this, SLOT(onTransferProgressChanged()));
    connect(transfer, SIGNAL(sizeChanged()), this, SLOT(onTransferSizeChanged()));
    connect(transfer, SIGNAL(statusChanged()), this, SLOT(onTransferStatusChanged()));
#else
    connect(transfer, SIGNAL(nameChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(categoryChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(priorityChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(progressChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(sizeChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(statusChanged()), this, SLOT(onTransferDataChanged()));
#endif
}

#ifdef WIDGETS_UI
void TransferModel::onTransferDataChanged(Transfer* transfer, int column) {
    const int row = indexOf(transfer);
    
    if (row != -1) {
        const QModelIndex &idx = index(row, column);
        emit dataChanged(idx, idx);
    }
}

void TransferModel::onTransferNameChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        onTransferDataChanged(transfer, 0);
    }
}

void TransferModel::onTransferCategoryChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        onTransferDataChanged(transfer, 1);
    }
}

void TransferModel::onTransferPriorityChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        onTransferDataChanged(transfer, 2);
    }
}

void TransferModel::onTransferProgressChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        onTransferDataChanged(transfer, 3);
    }
}

void TransferModel::onTransferSizeChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        onTransferDataChanged(transfer, 3);
    }
}

void TransferModel::onTransferStatusChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        onTransferDataChanged(transfer, 4);
    }
}
#else
void TransferModel::onTransferDataChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        const int row = indexOf(transfer);
        
        if (row != -1) {
            const QModelIndex &idx = index(row);
            emit dataChanged(idx, idx);
        }
    }
}
#endif
