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

#include "subscription.h"
#include "database.h"
#include <QSqlQuery>
#include <QSqlError>

Subscription::Subscription(QObject *parent) :
    QObject(parent),
    m_id(-1),
    m_cacheSize(0),
    m_downloadEnclosures(false),
    m_sourceType(Url),
    m_status(Idle),
    m_updateInterval(0),
    m_unreadArticles(0)
{
    connect(Database::instance(), SIGNAL(articlesAdded(int, int)), this, SLOT(onArticlesAdded(int, int)));
    connect(Database::instance(), SIGNAL(articleDeleted(int, int)), this, SLOT(onArticleDeleted(int, int)));
    connect(Database::instance(), SIGNAL(articleRead(int, int, bool)), this, SLOT(onArticleRead(int, int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
            this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
    connect(Database::instance(), SIGNAL(subscriptionRead(int, bool)), this, SLOT(onSubscriptionRead(int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionUpdated(int)), this, SLOT(onSubscriptionUpdated(int)));
}

Subscription::Subscription(const QSqlQuery &query, QObject *parent) :
    QObject(parent),
    m_id(Database::subscriptionId(query)),
    m_cacheSize(Database::subscriptionCacheSize(query)),
    m_description(Database::subscriptionDescription(query)),
    m_downloadEnclosures(Database::subscriptionDownloadEnclosures(query)),
    m_iconPath(Database::subscriptionIconPath(query)),
    m_lastUpdated(Database::subscriptionLastUpdated(query)),
    m_source(Database::subscriptionSource(query)),
    m_sourceType(SourceType(Database::subscriptionSourceType(query))),
    m_status(Ready),
    m_title(Database::subscriptionTitle(query)),
    m_updateInterval(Database::subscriptionUpdateInterval(query)),
    m_url(Database::subscriptionUrl(query)),
    m_unreadArticles(Database::subscriptionUnreadArticles(query))
{
    connect(Database::instance(), SIGNAL(articlesAdded(int, int)), this, SLOT(onArticlesAdded(int, int)));
    connect(Database::instance(), SIGNAL(articleDeleted(int, int)), this, SLOT(onArticleDeleted(int, int)));
    connect(Database::instance(), SIGNAL(articleRead(int, int, bool)), this, SLOT(onArticleRead(int, int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionRead(int, bool)), this, SLOT(onSubscriptionRead(int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionUpdated(int)), this, SLOT(onSubscriptionUpdated(int)));
}

Subscription::Subscription(int id, int cacheSize, const QString &description, bool downloadEnclosures,
                           const QString &iconPath, const QDateTime &lastUpdated, const QVariant &source,
                           SourceType sourceType, const QString &title, int updateInterval, const QUrl &url,
                           int unreadArticles, QObject *parent) :
    QObject(parent),
    m_id(id),
    m_cacheSize(cacheSize),
    m_description(description),
    m_downloadEnclosures(downloadEnclosures),
    m_iconPath(iconPath),
    m_lastUpdated(lastUpdated),
    m_source(source),
    m_sourceType(sourceType),
    m_status(Ready),
    m_title(title),
    m_updateInterval(updateInterval),
    m_url(url),
    m_unreadArticles(unreadArticles)
{
    connect(Database::instance(), SIGNAL(articlesAdded(int, int)), this, SLOT(onArticlesAdded(int, int)));
    connect(Database::instance(), SIGNAL(articleDeleted(int, int)), this, SLOT(onArticleDeleted(int, int)));
    connect(Database::instance(), SIGNAL(articleRead(int, int, bool)), this, SLOT(onArticleRead(int, int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionRead(int, bool)), this, SLOT(onSubscriptionRead(int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionUpdated(int)), this, SLOT(onSubscriptionUpdated(int)));
}

int Subscription::id() const {
    return m_id;
}

void Subscription::setId(int i) {
    if (i != id()) {
        m_id = i;
        emit idChanged();
        emit dataChanged(this);
    }
}

int Subscription::cacheSize() const {
    return m_cacheSize;
}

void Subscription::setCacheSize(int s) {
    if (s != cacheSize()) {
        m_cacheSize = s;
        emit cacheSizeChanged();
        emit dataChanged(this);
    }
}

QString Subscription::description() const {
    return m_description;
}

void Subscription::setDescription(const QString &d) {
    if (d != description()) {
        m_description = d;
        emit descriptionChanged();
        emit dataChanged(this);
    }
}

bool Subscription::downloadEnclosures() const {
    return m_downloadEnclosures;
}

void Subscription::setDownloadEnclosures(bool d) {
    if (d != downloadEnclosures()) {
        m_downloadEnclosures = d;
        emit downloadEnclosuresChanged();
        emit dataChanged(this);
    }
}

QString Subscription::errorString() const {
    return m_errorString;
}

void Subscription::setErrorString(const QString &e) {
    m_errorString = e;
}

QString Subscription::iconPath() const {
    return m_iconPath;
}

void Subscription::setIconPath(const QString &p) {
    if (p != iconPath()) {
        m_iconPath = p;
        emit iconPathChanged();
        emit dataChanged(this);
    }
}

QDateTime Subscription::lastUpdated() const {
    return m_lastUpdated;
}

void Subscription::setLastUpdated(const QDateTime &d) {
    if (d != lastUpdated()) {
        m_lastUpdated = d;
        emit lastUpdatedChanged();
        emit dataChanged(this);
    }
}

bool Subscription::isRead() const {
    return unreadArticles() == 0;
}

QVariant Subscription::source() const {
    return m_source;
}

void Subscription::setSource(const QVariant &s) {
    if (s != source()) {
        m_source = s;
        emit sourceChanged();
        emit dataChanged(this);
    }
}

Subscription::SourceType Subscription::sourceType() const {
    return m_sourceType;
}

void Subscription::setSourceType(Subscription::SourceType t) {
    if (t != sourceType()) {
        m_sourceType = t;
        emit sourceTypeChanged();
        emit dataChanged(this);
    }
}

Subscription::Status Subscription::status() const {
    return m_status;
}

void Subscription::setStatus(Subscription::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged();
    }
}

QString Subscription::title() const {
    return m_title;
}

void Subscription::setTitle(const QString &t) {
    if (t != title()) {
        m_title = t;
        emit titleChanged();
        emit dataChanged(this);
    }
}

int Subscription::updateInterval() const {
    return m_updateInterval;
}

void Subscription::setUpdateInterval(int i) {
    if (i != updateInterval()) {
        m_updateInterval = i;
        emit updateIntervalChanged();
        emit dataChanged(this);
    }
}

QUrl Subscription::url() const {
    return m_url;
}

void Subscription::setUrl(const QUrl &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged();
        emit dataChanged(this);
    }
}

int Subscription::unreadArticles() const {
    return m_unreadArticles;
}

void Subscription::setUnreadArticles(int u) {
    if ((u != unreadArticles()) && (u >= 0)) {
        m_unreadArticles = u;
        emit unreadArticlesChanged();
        emit dataChanged(this);
    }
}

void Subscription::load(int id) {
    setId(id);
    setStatus(Active);
    connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
            this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
    Database::fetchSubscription(id, id);
}

void Subscription::onArticlesAdded(int count, int subscriptionId) {
    if (subscriptionId == id()) {
        setUnreadArticles(unreadArticles() + count);
    }
}

void Subscription::onArticleDeleted(int, int subscriptionId) {
    if (subscriptionId == id()) {
        setStatus(Active);
        connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
        Database::fetchSubscription(id(), id());
    }
}

void Subscription::onArticleRead(int, int subscriptionId, bool isRead) {
    if (subscriptionId == id()) {
        setUnreadArticles(isRead ? unreadArticles() - 1 : unreadArticles() + 1);
    }
}

void Subscription::onSubscriptionFetched(const QSqlQuery &query, int requestId) {
    if (requestId == id()) {
        disconnect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                   this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
        
        setCacheSize(Database::subscriptionCacheSize(query)),
        setDescription(Database::subscriptionDescription(query));
        setDownloadEnclosures(Database::subscriptionDownloadEnclosures(query));
        setIconPath(Database::subscriptionIconPath(query));
        setLastUpdated(Database::subscriptionLastUpdated(query));
        setSource(Database::subscriptionSource(query));
        setSourceType(SourceType(Database::subscriptionSourceType(query)));
        setTitle(Database::subscriptionTitle(query));
        setUpdateInterval(Database::subscriptionUpdateInterval(query));
        setUrl(Database::subscriptionUrl(query));
        setUnreadArticles(Database::subscriptionUnreadArticles(query));
        setErrorString(query.lastError().text());
        setStatus(query.lastError().isValid() ? Error : Ready);
    }
}

void Subscription::onSubscriptionRead(int subscriptionId, bool isRead) {
    if (subscriptionId == id()) {
        if (isRead) {
            setUnreadArticles(0);
        }
        else {
            setStatus(Active);
            connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                    this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
            Database::fetchSubscription(subscriptionId, subscriptionId);
        }
    }
}

void Subscription::onSubscriptionUpdated(int subscriptionId) {
    if (subscriptionId == id()) {
        setStatus(Active);
        connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
        Database::fetchSubscription(subscriptionId, subscriptionId);
    }
}
