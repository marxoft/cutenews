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
        DownloadPathRole,
        ErrorStringRole,
        FileNameRole,
        IdRole,
        PriorityRole,
        PriorityStringRole,
        ProgressRole,
        ResourceIdRole,
        SizeRole,
        StatusRole,
        StatusStringRole,
        TransferTypeRole,
        UrlRole
    };
    
    explicit TransferModel(QObject *parent = 0);
    
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
#ifdef WIDGETS_UI
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
#endif
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
    Q_INVOKABLE QVariantMap itemData(int row) const;
    Q_INVOKABLE bool setData(int row, const QVariant &value, const QByteArray &role);
    Q_INVOKABLE bool setItemData(int row, const QVariantMap &roles);
    
    Q_INVOKABLE int match(const QByteArray &role, const QVariant &value) const;
    
private:
    int indexOf(Transfer *transfer) const;
    
private Q_SLOTS:
    void onCountChanged(int count);
    void onTransferAdded(Transfer *transfer);
#ifdef WIDGETS_UI
    void onTransferDataChanged(Transfer *transfer, int column);
    void onTransferFileNameChanged();
    void onTransferPriorityChanged();
    void onTransferProgressChanged();
    void onTransferSizeChanged();
    void onTransferStatusChanged();
#else
    void onTransferDataChanged();
#endif
    
Q_SIGNALS:
    void countChanged(int c);
    
private:
    QHash<int, QByteArray> m_roles;
};

#endif // TRANSFERMODEL_H
