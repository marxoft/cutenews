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

#include "subscriptionmodel.h"
#include "database.h"
#include "definitions.h"
#include "subscription.h"
#include <QSqlError>

SubscriptionModel::SubscriptionModel(QObject *parent) :
    QAbstractListModel(parent),
    m_status(Idle)
{
    m_roles[IdRole] = "id";
    m_roles[CacheSizeRole] = "cacheSize";
    m_roles[DescriptionRole] = "description";
    m_roles[DownloadEnclosuresRole] = "downloadEnclosures";
    m_roles[IconPathRole] = "iconPath";
    m_roles[LastUpdatedRole] = "lastUpdated";
    m_roles[ReadRole] = "read";
    m_roles[SourceRole] = "source";
    m_roles[SourceTypeRole] = "sourceType";
    m_roles[TitleRole] = "title";
    m_roles[UnreadArticlesRole] = "unreadArticles";
    m_roles[UpdateIntervalRole] = "updateInterval";
    m_roles[UrlRole] = "url";
#if QT_VERSION <= 0x050000
    setRoleNames(m_roles);
#endif
    connect(Database::instance(), SIGNAL(subscriptionAdded(int)), this, SLOT(onSubscriptionAdded(int)));
    connect(Database::instance(), SIGNAL(subscriptionDeleted(int)), this, SLOT(onSubscriptionDeleted(int)));
    connect(Database::instance(), SIGNAL(subscriptionsFetched(QSqlQuery, int)),
            this, SLOT(onSubscriptionsFetched(QSqlQuery, int)));
}

QString SubscriptionModel::errorString() const {
    return m_errorString;
}

void SubscriptionModel::setErrorString(const QString &e) {
    m_errorString = e;
}

SubscriptionModel::Status SubscriptionModel::status() const {
    return m_status;
}

void SubscriptionModel::setStatus(SubscriptionModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> SubscriptionModel::roleNames() const {
    return m_roles;
}
#endif

int SubscriptionModel::rowCount(const QModelIndex &) const {
    return m_list.size();
}

QVariant SubscriptionModel::data(const QModelIndex &index, int role) const {
    if (Subscription *subscription = get(index.row())) {
        return subscription->property(m_roles.value(role));
    }
    
    return QVariant();
}

QVariant SubscriptionModel::data(int row, const QByteArray &role) const {
    if (Subscription *subscription = get(row)) {
        return subscription->property(role);
    }
    
    return QVariant();
}

QMap<int, QVariant> SubscriptionModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
        
    if (Subscription *subscription = get(index.row())) {
        QHashIterator<int, QByteArray> iterator(m_roles);
        
        while (iterator.hasNext()) {
            iterator.next();
            map[iterator.key()] = subscription->property(iterator.value());
        }
    }
    
    return map;
}

QVariantMap SubscriptionModel::itemData(int row) const {
    QVariantMap map;
        
    if (Subscription *subscription = get(row)) {
        foreach (QByteArray role, m_roles.values()) {
            map[role] = subscription->property(role);
        }
    }
    
    return map;
}

Subscription* SubscriptionModel::get(int row) const {
    if ((row >= 0) && (row < m_list.size())) {
        return m_list.at(row);
    }
    
    return 0;
}

int SubscriptionModel::match(const QByteArray &role, const QVariant &value) const {
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list.at(i)->property(role) == value) {
            return i;
        }
    }
    
    return -1;
}

void SubscriptionModel::clear() {
    if (!m_list.isEmpty()) {
        beginResetModel();
        qDeleteAll(m_list);
        m_list.clear();
        endResetModel();
        emit countChanged(0);
        setStatus(Idle);
    }
}

void SubscriptionModel::load() {
    clear();
    beginInsertRows(QModelIndex(), 0, 1);
    m_list << new Subscription(ALL_ARTICLES_SUBSCRIPTION_ID, 0, tr("All articles"), false, QString(), QDateTime(),
                               QString(), Subscription::None, tr("All articles"), 0, QUrl(), 0, this);
    m_list << new Subscription(FAVOURITES_SUBSCRIPTION_ID, 0, tr("Favourites"), false, QString(), QDateTime(),
                               QString(), Subscription::None, tr("Favourites"), 0, QUrl(), 0, this);
    endInsertRows();
    emit countChanged(rowCount());
    setStatus(Active);
    Database::fetchSubscriptions(-1);
}

void SubscriptionModel::onSubscriptionChanged(Subscription *subscription) {
    const int i = m_list.indexOf(subscription);
    
    if (i >= 0) {
        const QModelIndex idx = index(i);
        emit dataChanged(idx, idx);
    }
}

void SubscriptionModel::onSubscriptionAdded(int id) {
    if (status() == Ready) {
        setStatus(Active);
        Database::fetchSubscriptions("WHERE subscriptions.id = " + QString::number(id), -1);
    }
}

void SubscriptionModel::onSubscriptionDeleted(int id) {
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list.at(i)->id() == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_list.takeAt(i)->deleteLater();
            endRemoveRows();
            emit countChanged(rowCount());
            return;
        }
    }
}

void SubscriptionModel::onSubscriptionsFetched(QSqlQuery query, int requestId) {
    if (requestId == -1) {
        if (query.lastError().isValid()) {
            setErrorString(query.lastError().text());
            setStatus(Error);
            return;
        }
        
        while (query.next()) {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            Subscription *subscription = new Subscription(query, this);
            connect(subscription, SIGNAL(dataChanged(Subscription*)), this, SLOT(onSubscriptionChanged(Subscription*)));
            m_list << subscription;
            endInsertRows();
        }
        
        emit countChanged(rowCount());
        setStatus(Ready);
    }
}
