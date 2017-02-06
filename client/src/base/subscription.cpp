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
#include "dbconnection.h"
#include "dbnotify.h"
#include "json.h"

class SubscriptionRoleNames : public QHash<int, QByteArray>
{

public:
    SubscriptionRoleNames() :
        QHash<int, QByteArray>()
    {
        insert(Subscription::AutoUpdateRole, "autoUpdate");
        insert(Subscription::DescriptionRole, "description");
        insert(Subscription::DownloadEnclosuresRole, "downloadEnclosures");
        insert(Subscription::ErrorStringRole, "errorString");
        insert(Subscription::IconPathRole, "iconPath");
        insert(Subscription::IdRole, "id");
        insert(Subscription::LastUpdatedRole, "lastUpdated");
        insert(Subscription::LastUpdatedStringRole, "lastUpdatedString");
        insert(Subscription::ReadRole, "read");
        insert(Subscription::SourceRole, "source");
        insert(Subscription::SourceTypeRole, "sourceType");
        insert(Subscription::StatusRole, "status");
        insert(Subscription::TitleRole, "title");
        insert(Subscription::UnreadArticlesRole, "unreadArticles");
        insert(Subscription::UpdateIntervalRole, "updateInterval");
        insert(Subscription::UrlRole, "url");
    }
};

QHash<int, QByteArray> Subscription::roles = SubscriptionRoleNames();

Subscription::Subscription(QObject *parent) :
    QObject(parent),
    m_downloadEnclosures(false),
    m_sourceType(Url),
    m_status(Idle),
    m_updateInterval(0),
    m_unreadArticles(0),
    m_autoUpdate(false)
{
}

Subscription::Subscription(const QString &id, const QString &description, bool downloadEnclosures,
                           const QString &iconPath, const QDateTime &lastUpdated, const QVariant &source,
                           SourceType sourceType, const QString &title, int updateInterval, const QString &url,
                           int unreadArticles, QObject *parent) :
    QObject(parent),
    m_id(id),
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
    m_unreadArticles(unreadArticles),
    m_autoUpdate(false)
{
}

QHash<int, QByteArray> Subscription::roleNames() {
    return roles;
}

QVariant Subscription::data(int role) const {
    switch (role) {
    case AutoUpdateRole:
        return autoUpdate();
    case DescriptionRole:
        return description();
    case DownloadEnclosuresRole:
        return downloadEnclosures();
    case IconPathRole:
        return iconPath();
    case IdRole:
        return id();
    case LastUpdatedRole:
        return lastUpdated();
    case LastUpdatedStringRole:
        return lastUpdatedString();
    case ReadRole:
        return isRead();
    case SourceRole:
        return source();
    case SourceTypeRole:
        return sourceType();
    case StatusRole:
        return status();
    case TitleRole:
        return title();
    case UnreadArticlesRole:
        return unreadArticles();
    case UpdateIntervalRole:
        return updateInterval();
    case UrlRole:
        return url();
    default:
        return QVariant();
    }
}

QVariant Subscription::data(const QByteArray &roleName) const {
    return data(roles.key(roleName));
}

bool Subscription::setData(int role, const QVariant &value) {
    switch (role) {
    case AutoUpdateRole:
        setAutoUpdate(value.toBool());
        return true;
    case ReadRole:
        markRead(value.toBool());
        return true;
    default:
        return false;
    }
}

bool Subscription::setData(const QByteArray &roleName, const QVariant &value) {
    return setData(roles.key(roleName), value);
}

QString Subscription::id() const {
    return m_id;
}

void Subscription::setId(const QString &i) {
    if (i != id()) {
        m_id = i;
        emit idChanged();
        emit dataChanged(this, IdRole);
    }
}

QString Subscription::description() const {
    return m_description;
}

void Subscription::setDescription(const QString &d) {
    if (d != description()) {
        m_description = d;
        emit descriptionChanged();
        emit dataChanged(this, DescriptionRole);
    }
}

bool Subscription::downloadEnclosures() const {
    return m_downloadEnclosures;
}

void Subscription::setDownloadEnclosures(bool d) {
    if (d != downloadEnclosures()) {
        m_downloadEnclosures = d;
        emit downloadEnclosuresChanged();
        emit dataChanged(this, DownloadEnclosuresRole);
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
        emit dataChanged(this, IconPathRole);
    }
}

QDateTime Subscription::lastUpdated() const {
    return m_lastUpdated;
}

void Subscription::setLastUpdated(const QDateTime &d) {
    if (d != lastUpdated()) {
        m_lastUpdated = d;
        emit lastUpdatedChanged();
        emit dataChanged(this, LastUpdatedRole);
    }
}

QString Subscription::lastUpdatedString() const {
    return lastUpdated().toString("dd MMM yyyy HH:mm");
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
        emit dataChanged(this, SourceRole);
    }
}

Subscription::SourceType Subscription::sourceType() const {
    return m_sourceType;
}

void Subscription::setSourceType(Subscription::SourceType t) {
    if (t != sourceType()) {
        m_sourceType = t;
        emit sourceTypeChanged();
        emit dataChanged(this, SourceTypeRole);
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
        emit dataChanged(this, TitleRole);
    }
}

int Subscription::updateInterval() const {
    return m_updateInterval;
}

void Subscription::setUpdateInterval(int i) {
    if (i != updateInterval()) {
        m_updateInterval = i;
        emit updateIntervalChanged();
        emit dataChanged(this, UpdateIntervalRole);
    }
}

QString Subscription::url() const {
    return m_url;
}

void Subscription::setUrl(const QString &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged();
        emit dataChanged(this, UrlRole);
    }
}

int Subscription::unreadArticles() const {
    return m_unreadArticles;
}

void Subscription::setUnreadArticles(int u) {
    if ((u != unreadArticles()) && (u >= 0)) {
        m_unreadArticles = u;
        emit unreadArticlesChanged();
        emit dataChanged(this, UnreadArticlesRole);
    }
}

bool Subscription::autoUpdate() const {
    return m_autoUpdate;
}

void Subscription::setAutoUpdate(bool enabled) {
    if (enabled != autoUpdate()) {
        m_autoUpdate = enabled;
        emit autoUpdateChanged();
        emit dataChanged(this, AutoUpdateRole);

        if (enabled) {
            connect(DBNotify::instance(), SIGNAL(articleRead(QString, QString, bool, QVariantMap)),
                    this, SLOT(onArticleRead(QString, QString, bool)));
            connect(DBNotify::instance(), SIGNAL(subscriptionRead(QString, bool, QVariantMap)),
                    this, SLOT(onSubscriptionRead(QString, bool, QVariantMap)));
            connect(DBNotify::instance(), SIGNAL(allSubscriptionsRead()), this, SLOT(onAllSubscriptionsRead()));
            connect(DBNotify::instance(), SIGNAL(subscriptionUpdated(QString, QVariantMap)),
                    this, SLOT(onSubscriptionUpdated(QString, QVariantMap)));
        }
        else {
            disconnect(DBNotify::instance(), 0, this, 0);
        }
    }
}

void Subscription::load(const QString &id) {
    setId(id);
    setStatus(Active);
    DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)))->fetchSubscription(id);
}

void Subscription::markRead(bool read) {
    if (read != isRead()) {
        DBConnection *connection = DBConnection::connection();
        connect(connection, SIGNAL(finished(DBConnection*)), connection, SLOT(deleteLater()));
        connection->markSubscriptionRead(id(), read);
    }
}

void Subscription::remove() {
    DBConnection *connection = DBConnection::connection();
    connect(connection, SIGNAL(finished(DBConnection*)), connection, SLOT(deleteLater()));
    connection->deleteSubscription(id());
}

void Subscription::update(const QVariantMap &properties) {
    DBConnection *connection = DBConnection::connection();
    connect(connection, SIGNAL(finished(DBConnection*)), connection, SLOT(deleteLater()));
    connection->updateSubscription(id(), properties);
}

void Subscription::load(const QVariantMap &properties) {
    setDescription(properties.value("description").toString());
    setDownloadEnclosures(properties.value("downloadEnclosures").toBool());
    setIconPath(properties.value("iconPath").toString());
    setLastUpdated(QDateTime::fromTime_t(properties.value("lastUpdated").toUInt()));
    setSourceType(SourceType(properties.value("sourceType").toInt()));
    setSource(properties.value("source"));
    setTitle(properties.value("title").toString());
    setUpdateInterval(properties.value("updateInterval").toInt());
    setUrl(properties.value("url").toString());
    setUnreadArticles(properties.value("unreadArticles").toInt());
}

void Subscription::onArticleRead(const QString &, const QString &subscriptionId, bool isRead) {
    if (subscriptionId == id()) {
        setUnreadArticles(isRead ? unreadArticles() - 1 : unreadArticles() + 1);
    }
}

void Subscription::onSubscriptionFetched(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {
        const QVariantMap result = connection->result().toMap();
        load(result);
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(connection->errorString());
        setStatus(Error);
    }
    
    connection->deleteLater();
    emit finished(this);
}

void Subscription::onSubscriptionRead(const QString &subscriptionId, bool, const QVariantMap &properties) {
    onSubscriptionUpdated(subscriptionId, properties);
}

void Subscription::onAllSubscriptionsRead() {
    setUnreadArticles(0);
}

void Subscription::onSubscriptionUpdated(const QString &subscriptionId, const QVariantMap &properties) {
    if (subscriptionId == id()) {
        load(properties);
    }
}
