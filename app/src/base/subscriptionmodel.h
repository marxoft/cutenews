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

#ifndef SUBSCRIPTIONMODEL_H
#define SUBSCRIPTIONMODEL_H

#include <QAbstractListModel>

class DBConnection;
class Subscription;

class SubscriptionModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)
    
public:
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Error
    };
    
    explicit SubscriptionModel(QObject *parent = 0);
    
    QString errorString() const;
    
    Status status() const;
    
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    
    QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Q_INVOKABLE bool setData(int row, const QVariant &value, const QByteArray &role);
    
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE QVariantMap itemData(int row) const;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    Q_INVOKABLE bool setItemData(int row, const QVariantMap &roles);
    
    Q_INVOKABLE Subscription* get(int row) const;
    Q_INVOKABLE bool remove(int row);
    
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
                          Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    Q_INVOKABLE int match(int start, const QByteArray &role, const QVariant &value,
                          int flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;

public Q_SLOTS:
    void clear();
    void load();

private Q_SLOTS:
    void onSubscriptionChanged(Subscription *subscription, int role);
    void onSubscriptionsAdded(const QStringList &ids);
    void onSubscriptionDeleted(const QString &id);
    void onSubscriptionsFetched(DBConnection *connection);

Q_SIGNALS:
    void countChanged(int count);
    void statusChanged(SubscriptionModel::Status status);
    
private:
    void setErrorString(const QString &e);
    
    void setStatus(Status s);
    
    QList<Subscription*> m_list;
        
    QString m_errorString;
    
    Status m_status;
};

#endif // SUBSCRIPTIONMODEL_H
