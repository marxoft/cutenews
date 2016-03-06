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
#include <QSqlQuery>

class Subscription;

class SubscriptionModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)
    
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        CacheSizeRole,
        DescriptionRole,
        DownloadEnclosuresRole,
        IconPathRole,
        LastUpdatedRole,
        ReadRole,
        SourceRole,
        SourceTypeRole,
        TitleRole,
        UnreadArticlesRole,
        UpdateIntervalRole,
        UrlRole
    };
    
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
#ifdef WIDGETS_UI
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
#endif
    QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
    
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE QVariantMap itemData(int row) const;
    
    Q_INVOKABLE Subscription* get(int row) const;
    
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
                          Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    Q_INVOKABLE int match(int start, const QByteArray &role, const QVariant &value,
                          int flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;

public Q_SLOTS:
    void clear();
    void load();

private:
    void setErrorString(const QString &e);
    
    void setStatus(Status s);

private Q_SLOTS:
    void onSubscriptionChanged(Subscription *subscription);
    void onSubscriptionsAdded(int count);
    void onSubscriptionDeleted(int id);
    void onSubscriptionsFetched(QSqlQuery query, int requestId);

Q_SIGNALS:
    void countChanged(int count);
    void statusChanged(SubscriptionModel::Status status);
    
private:
    QList<Subscription*> m_list;
    
    QHash<int, QByteArray> m_roles;
    
    QString m_errorString;
    
    Status m_status;
};

#endif // SUBSCRIPTIONMODEL_H
