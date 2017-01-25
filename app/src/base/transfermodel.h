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

#ifndef TRANSFERMODEL_H
#define TRANSFERMODEL_H

#include <QAbstractListModel>

class Transfer;

class TransferModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    
public:
    enum Roles {
        BytesTransferredRole = Qt::UserRole + 1,
        CategoryRole,
        CustomCommandRole,
        CustomCommandOverrideEnabledRole,
        DownloadPathRole,
        ErrorStringRole,
        FileNameRole,
        IdRole,
        NameRole,
        PriorityRole,
        PriorityStringRole,
        ProgressRole,
        ProgressStringRole,
        SizeRole,
        SizeStringRole,
        SpeedRole,
        SpeedStringRole,
        StatusRole,
        StatusStringRole,
        SubscriptionIdRole,
        TransferTypeRole,
        UrlRole
    };
    
    explicit TransferModel(QObject *parent = 0);
    
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
    Q_INVOKABLE QVariantMap itemData(int row) const;
    Q_INVOKABLE bool setData(int row, const QVariant &value, const QByteArray &role);
    Q_INVOKABLE bool setItemData(int row, const QVariantMap &roles);
    
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
                          Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    Q_INVOKABLE int match(int start, const QByteArray &role, const QVariant &value,
                          int flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    
private:
    int indexOf(Transfer *transfer) const;
    
private Q_SLOTS:
    void onCountChanged(int count);
    void onTransferAdded(Transfer *transfer);
    void onTransferDataChanged(Transfer *transfer, int role);
    
Q_SIGNALS:
    void countChanged(int count);
    
private:
    QHash<int, QByteArray> m_roles;
};

#endif // TRANSFERMODEL_H
