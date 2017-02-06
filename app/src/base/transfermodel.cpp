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

TransferModel::TransferModel(QObject *parent) :
    QAbstractListModel(parent)
{
#if QT_VERSION < 0x050000
    setRoleNames(Transfer::roleNames());
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
    return Transfer::roleNames();
}
#endif

int TransferModel::rowCount(const QModelIndex &) const {
    return Transfers::instance()->count();
}

int TransferModel::columnCount(const QModelIndex &) const {
    return 5;
}

QVariant TransferModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Priority");
        case 2:
            return tr("Progress");
        case 3:
            return tr("Speed");
        case 4:
            return tr("Status");
        default:
            break;
        }
    }
    
    return QVariant();
}

QVariant TransferModel::data(const QModelIndex &index, int role) const {
    if (const Transfer *transfer = Transfers::instance()->get(index.row())) {
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0:
                return transfer->data(Transfer::NameRole);
            case 1:
                return transfer->data(Transfer::PriorityStringRole);
            case 2:
                return transfer->data(Transfer::ProgressStringRole);
            case 3:
                return transfer->data(Transfer::SpeedStringRole);
            case 4:
                return transfer->data(Transfer::StatusStringRole);
            default:
                return QVariant();
            }
        }
        
        return transfer->data(role);
    }
    
    return QVariant();
}

QMap<int, QVariant> TransferModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    
    if (const Transfer *transfer = Transfers::instance()->get(index.row())) {
        for (int i = Transfer::BytesTransferredRole; i <= Transfer::UsePluginRole; i++) {
            map[i] = transfer->data(i);
        }
    }
    
    return map;
}

bool TransferModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (Transfer *transfer = Transfers::instance()->get(index.row())) {
        return transfer->setData(role, value);
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
    return data(index(row), roleNames().key(role));
}

QVariantMap TransferModel::itemData(int row) const {
    QVariantMap map;
    
    if (const Transfer *transfer = Transfers::instance()->get(row)) {
        for (int i = Transfer::BytesTransferredRole; i <= Transfer::UsePluginRole; i++) {
            map[roleNames().value(i)] = transfer->data(i);
        }
    }

    return map;
}

bool TransferModel::setData(int row, const QVariant &value, const QByteArray &role) {
    return setData(index(row), value, roleNames().key(role));
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
    const QModelIndexList indexes = match(index(start), roleNames().key(role), value, Qt::MatchFlags(flags));
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
    connect(transfer, SIGNAL(dataChanged(Transfer*, int)), this, SLOT(onTransferDataChanged(Transfer*, int)));
}

void TransferModel::onTransferDataChanged(Transfer* transfer, int role) {
    const int row = indexOf(transfer);

    if (row == -1) {
        return;
    }

    int column = 0;

    switch (role) {
    case Transfer::BytesTransferredRole:
    case Transfer::ProgressRole:
    case Transfer::SizeRole:
        column = 2;
        break;
    case Transfer::SpeedRole:
        column = 3;
        break;
    case Transfer::StatusRole:
        column = 4;
        break;
    case Transfer::PriorityRole:
        column = 1;
        break;
    default:
        break;
    }

    const QModelIndex idx = index(row, column);
    emit dataChanged(idx, idx);
}
