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
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "json.h"
#include "subscription.h"
#include <QFont>
#include <QIcon>

SubscriptionModel::SubscriptionModel(QObject *parent) :
    QAbstractListModel(parent),
    m_status(Idle)
{
#if QT_VERSION <= 0x050000
    setRoleNames(Subscription::roleNames());
#endif
    connect(DBNotify::instance(), SIGNAL(subscriptionsAdded(QStringList)),
            this, SLOT(onSubscriptionsAdded(QStringList)));
    connect(DBNotify::instance(), SIGNAL(subscriptionDeleted(QString)), this, SLOT(onSubscriptionDeleted(QString)));
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
    return Subscription::roleNames();
}
#endif

int SubscriptionModel::rowCount(const QModelIndex &) const {
    return m_list.size();
}

int SubscriptionModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant SubscriptionModel::data(const QModelIndex &index, int role) const {
    if (const Subscription *subscription = get(index.row())) {
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
        
        return subscription->data(role);
    }
    
    return QVariant();
}

QVariant SubscriptionModel::data(int row, const QByteArray &role) const {
    return data(index(row), roleNames().key(role));
}

bool SubscriptionModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (Subscription *subscription = get(index.row())) {
        return subscription->setData(role, value);
    }

    return false;
}

bool SubscriptionModel::setData(int row, const QVariant &value, const QByteArray &role) {
    return setData(index(row), value, roleNames().key(role));
}

QMap<int, QVariant> SubscriptionModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    
    if (const Subscription *subscription = get(index.row())) {
        for (int i = Subscription::AutoUpdateRole; i <= Subscription::UrlRole; i++) {
            map[i] = subscription->data(i);
        }
    }
    
    return map;
}

QVariantMap SubscriptionModel::itemData(int row) const {
    QVariantMap map;
    
    if (const Subscription *subscription = get(row)) {
        for (int i = Subscription::AutoUpdateRole; i <= Subscription::UrlRole; i++) {
            map[roleNames().value(i)] = subscription->data(i);
        }
    }
    
    return map;
}

bool SubscriptionModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    QMapIterator<int, QVariant> iterator(roles);

    while (iterator.hasNext()) {
        iterator.next();

        if (!setData(index, iterator.value(), iterator.key())) {
            return false;
        }
    }

    return true;
}

bool SubscriptionModel::setItemData(int row, const QVariantMap &roles) {
    QMapIterator<QString, QVariant> iterator(roles);

    while (iterator.hasNext()) {
        iterator.next();

        if (!setData(row, iterator.value(), iterator.key().toUtf8())) {
            return false;
        }
    }

    return true;
}

Subscription* SubscriptionModel::get(int row) const {
    if ((row >= 0) && (row < m_list.size())) {
        return m_list.at(row);
    }
    
    return 0;
}

bool SubscriptionModel::remove(int row) {
    if (Subscription *subscription = get(row)) {
        subscription->remove();
        return true;
    }

    return false;
}

QModelIndexList SubscriptionModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                         Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int SubscriptionModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList indexes = match(index(start), roleNames().key(role), value, Qt::MatchFlags(flags));
    return indexes.isEmpty() ? -1 : indexes.first().row();
}

void SubscriptionModel::clear() {
    if (!m_list.isEmpty()) {
        beginResetModel();
        qDeleteAll(m_list);
        m_list.clear();
        endResetModel();
        emit countChanged(0);
    }

    setStatus(Idle);
}

void SubscriptionModel::load() {
    if (status() == Active) {
        return;
    }

    clear();
    setStatus(Active);
    beginInsertRows(QModelIndex(), 0, 1);
    m_list << new Subscription(ALL_ARTICLES_SUBSCRIPTION_ID, tr("All articles"), false, QString(), QDateTime(),
                               QString(), Subscription::None, tr("All articles"), 0, QString(), 0, this);
    m_list << new Subscription(FAVOURITES_SUBSCRIPTION_ID, tr("Favourites"), false, QString(), QDateTime(),
                               QString(), Subscription::None, tr("Favourites"), 0, QString(), 0, this);
    endInsertRows();
    emit countChanged(rowCount());
    DBConnection::connection(this, SLOT(onSubscriptionsFetched(DBConnection*)))->fetchSubscriptions();
}

void SubscriptionModel::onSubscriptionChanged(Subscription *subscription, int) {
    const int row = m_list.indexOf(subscription);
    
    if (row == -1) {
        return;
    }
    
    emit dataChanged(index(row, 0), index(row, 1));
}

void SubscriptionModel::onSubscriptionsAdded(const QStringList &ids) {
    if (status() == Ready) {
        setStatus(Active);
        DBConnection::connection(this, SLOT(onSubscriptionsFetched(DBConnection*)))->fetchSubscriptions(ids);
    }
}

void SubscriptionModel::onSubscriptionDeleted(const QString &id) {
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

void SubscriptionModel::onSubscriptionsFetched(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {
        while (connection->nextRecord()) {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            Subscription *subscription = new Subscription(connection->value(0).toString(),
                                                          connection->value(1).toString(),
                                                          connection->value(2).toBool(),
                                                          connection->value(3).toString(),
                                                          QDateTime::fromTime_t(connection->value(4).toInt()),
                                                          QtJson::Json::parse(connection->value(5).toString()),
                                                          Subscription::SourceType(connection->value(6).toInt()),
                                                          connection->value(7).toString(),
                                                          connection->value(8).toInt(),
                                                          connection->value(9).toString(),
                                                          connection->value(10).toInt(), this);
            subscription->setAutoUpdate(true);
            connect(subscription, SIGNAL(dataChanged(Subscription*, int)),
                    this, SLOT(onSubscriptionChanged(Subscription*, int)));
            m_list << subscription;
            endInsertRows();
        }
        
        emit countChanged(rowCount());
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(connection->errorString());
        setStatus(Error);
    }
    
    connection->deleteLater();
}
