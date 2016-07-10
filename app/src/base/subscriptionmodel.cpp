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

#include "subscriptionmodel.h"
#include "database.h"
#include "definitions.h"
#include "subscription.h"
#include <QSqlError>
#ifdef WIDGETS_UI
#include <QFont>
#include <QIcon>
#endif

static const int REQUEST_ID = 1000;

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
    connect(Database::instance(), SIGNAL(subscriptionsAdded(int)), this, SLOT(onSubscriptionsAdded(int)));
    connect(Database::instance(), SIGNAL(subscriptionDeleted(int)), this, SLOT(onSubscriptionDeleted(int)));
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

#ifdef WIDGETS_UI
int SubscriptionModel::columnCount(const QModelIndex &) const {
    return 2;
}
#endif

QVariant SubscriptionModel::data(const QModelIndex &index, int role) const {
    if (Subscription *subscription = get(index.row())) {
#ifdef WIDGETS_UI
        switch (index.column()) {
        case 0:
            switch (role) {
            case Qt::DisplayRole:
                return subscription->title();
            case Qt::DecorationRole:
                return subscription->iconPath().isEmpty() ? QIcon::fromTheme("cutenews")
                                                          : QIcon(subscription->iconPath());
            case Qt::FontRole:
                if (subscription->unreadArticles() > 0) {
                    QFont font;
                    font.setBold(true);
                    return font;
                }
                
                return QFont();
            default:
                break;
            }
            
            break;
        case 1:
            switch (role) {
            case Qt::DisplayRole:
                if (subscription->unreadArticles() > 0) {
                    return subscription->unreadArticles();
                }
                
                break;
            case Qt::FontRole:
                if (subscription->unreadArticles() > 0) {
                    QFont font;
                    font.setBold(true);
                    return font;
                }
                
                return QFont();
            default:
                break;
            }
            
            break;
        default:
            break;
        }                
#endif
        return subscription->property(m_roles.value(role));
    }
    
    return QVariant();
}

QVariant SubscriptionModel::data(int row, const QByteArray &role) const {
    if (const Subscription *subscription = get(row)) {
        return subscription->property(role);
    }
    
    return QVariant();
}

QMap<int, QVariant> SubscriptionModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
        
    if (const Subscription *subscription = get(index.row())) {
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
        
    if (const Subscription *subscription = get(row)) {
        foreach (const QByteArray &role, m_roles.values()) {
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

QModelIndexList SubscriptionModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                         Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int SubscriptionModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList indexes = match(index(start), m_roles.key(role), value, Qt::MatchFlags(flags));
    return indexes.isEmpty() ? -1 : indexes.first().row();
}

void SubscriptionModel::clear() {
    if (!m_list.isEmpty()) {
        setStatus(Active);
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
    connect(Database::instance(), SIGNAL(subscriptionsFetched(QSqlQuery, int)),
            this, SLOT(onSubscriptionsFetched(QSqlQuery, int)));
    Database::fetchSubscriptions(REQUEST_ID);
}

void SubscriptionModel::onSubscriptionChanged(Subscription *subscription) {
    const int i = m_list.indexOf(subscription);
    
    if (i >= 0) {
#ifdef WIDGETS_UI
        emit dataChanged(index(i, 0), index(i, 1));
#else
        const QModelIndex idx = index(i);
        emit dataChanged(idx, idx);
#endif
    }
}

void SubscriptionModel::onSubscriptionsAdded(int count) {
    if (status() == Ready) {
        setStatus(Active);
        connect(Database::instance(), SIGNAL(subscriptionsFetched(QSqlQuery, int)),
                this, SLOT(onSubscriptionsFetched(QSqlQuery, int)));
        Database::fetchSubscriptions(QString("ORDER BY id DESC LIMIT %1").arg(count), REQUEST_ID);
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
    if (requestId == REQUEST_ID) {
        disconnect(Database::instance(), SIGNAL(subscriptionsFetched(QSqlQuery, int)),
                   this, SLOT(onSubscriptionsFetched(QSqlQuery, int)));
        
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
