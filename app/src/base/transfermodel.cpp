/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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

TransferModel::TransferModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_roles[BytesTransferredRole] = "bytesTransferred";
    m_roles[DownloadPathRole] = "downloadPath";
    m_roles[ErrorStringRole] = "errorString";
    m_roles[FileNameRole] = "fileName";
    m_roles[IdRole] = "id";
    m_roles[PriorityRole] = "priority";
    m_roles[PriorityStringRole] = "priorityString";
    m_roles[ProgressRole] = "progress";
    m_roles[SizeRole] = "size";
    m_roles[StatusRole] = "status";
    m_roles[StatusStringRole] = "statusString";
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

QVariant TransferModel::data(const QModelIndex &index, int role) const {
    if (Transfer *transfer = Transfers::instance()->get(index.row())) {
        return transfer->property(m_roles[role]);
    }
    
    return QVariant();
}

QMap<int, QVariant> TransferModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    
    if (Transfer *transfer = Transfers::instance()->get(index.row())) {
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
    bool ok = false;
    
    while (iterator.hasNext()) {
        iterator.next();
        ok = setData(index, iterator.value(), iterator.key());
        
        if (!ok) {
            return false;
        }
    }
    
    return ok;
}

QVariant TransferModel::data(int row, const QByteArray &role) const {
    if (Transfer *transfer = Transfers::instance()->get(row)) {
        return transfer->property(role);
    }

    return QVariant();
}

QVariantMap TransferModel::itemData(int row) const {
    QVariantMap map;
    
    if (Transfer *transfer = Transfers::instance()->get(row)) {
        foreach (QByteArray value, m_roles.values()) {
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
    bool ok = false;
    
    while (iterator.hasNext()) {
        iterator.next();
        ok = setData(row, iterator.value(), iterator.key().toUtf8());

        if (!ok) {
            return false;
        }
    }
    
    return ok;
}

int TransferModel::match(const QByteArray &role, const QVariant &value) const {
    for (int i = 0; i < Transfers::instance()->count(); i++) {
        if (data(i, role) == value) {
            return i;
        }
    }
    
    return -1;
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
    connect(transfer, SIGNAL(fileNameChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(priorityChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(progressChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(sizeChanged()), this, SLOT(onTransferDataChanged()));
    connect(transfer, SIGNAL(statusChanged()), this, SLOT(onTransferDataChanged()));
}

void TransferModel::onTransferDataChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        const int row = indexOf(transfer);
        
        if (row != -1) {
            const QModelIndex &idx = index(row);
            emit dataChanged(idx, idx);
        }
    }
}
