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

#include "article.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "json.h"

Article::Article(QObject *parent) :
    QObject(parent),
    m_favourite(false),
    m_read(false),
    m_status(Idle),
    m_autoUpdate(false)
{
}

Article::Article(const QString &id, const QString &author, const QString &body, const QStringList &categories,
                 const QDateTime &date, const QVariantList &enclosures, bool isFavourite, bool isRead,
                 const QString &subscriptionId, const QString &title, const QString &url, QObject *parent) :
    QObject(parent),
    m_id(id),
    m_author(author),
    m_body(body),
    m_categories(categories),
    m_date(date),
    m_enclosures(enclosures),
    m_favourite(isFavourite),
    m_read(isRead),
    m_status(Ready),
    m_subscriptionId(subscriptionId),
    m_title(title),
    m_url(url),
    m_autoUpdate(false)
{
}

QVariant Article::data(int role) const {
    switch (role) {
    case AuthorRole:
        return author();
    case AutoUpdateRole:
        return autoUpdate();
    case BodyRole:
        return body();
    case CategoriesRole:
        return categories();
    case DateRole:
        return date();
    case DateStringRole:
        return dateString();
    case EnclosuresRole:
        return enclosures();
    case FavouriteRole:
        return isFavourite();
    case HasEnclosuresRole:
        return hasEnclosures();
    case IdRole:
        return id();
    case ReadRole:
        return isRead();
    case StatusRole:
        return status();
    case SubscriptionIdRole:
        return subscriptionId();
    case TitleRole:
        return title();
    case UrlRole:
        return url();
    default:
        return QVariant();
    }
}

bool Article::setData(int role, const QVariant &value) {
    switch (role) {
    case AutoUpdateRole:
        setAutoUpdate(value.toBool());
        return true;
    case FavouriteRole:
        markFavourite(value.toBool());
        return true;
    case ReadRole:
        markRead(value.toBool());
        return true;
    default:
        return false;
    }
}

QString Article::id() const {
    return m_id;
}

void Article::setId(const QString &i) {
    if (i != id()) {
        m_id = i;
        emit idChanged();
        emit dataChanged(this, IdRole);
    }
}

QString Article::author() const {
    return m_author;
}

void Article::setAuthor(const QString &a) {
    if (a != author()) {
        m_author = a;
        emit authorChanged();
        emit dataChanged(this, AuthorRole);
    }
}

QString Article::body() const {
    return m_body;
}

void Article::setBody(const QString &b) {
    if (b != body()) {
        m_body = b;
        emit bodyChanged();
        emit dataChanged(this, BodyRole);
    }
}

QStringList Article::categories() const {
    return m_categories;
}

void Article::setCategories(const QStringList &c) {
    m_categories = c;
    emit categoriesChanged();
    emit dataChanged(this, CategoriesRole);
}

QDateTime Article::date() const {
    return m_date;
}

void Article::setDate(const QDateTime &d) {
    if (d != date()) {
        m_date = d;
        emit dateChanged();
        emit dataChanged(this, DateRole);
    }
}

QString Article::dateString() const {
    return date().toString("dd MMM yyyy HH:mm");
}

QVariantList Article::enclosures() const {
    return m_enclosures;
}

void Article::setEnclosures(const QVariantList &e) {
    m_enclosures = e;
    emit enclosuresChanged();
    emit dataChanged(this, EnclosuresRole);
}

QString Article::errorString() const {
    return m_errorString;
}

void Article::setErrorString(const QString &e) {
    m_errorString = e;
}

bool Article::hasEnclosures() const {
    return !m_enclosures.isEmpty();
}

bool Article::isFavourite() const {
    return m_favourite;
}

void Article::setFavourite(bool f) {
    if (f != isFavourite()) {
        m_favourite = f;
        emit favouriteChanged();
        emit dataChanged(this, FavouriteRole);
    }
}

bool Article::isRead() const {
    return m_read;
}

void Article::setRead(bool r) {
    if (r != isRead()) {
        m_read = r;
        emit readChanged();
        emit dataChanged(this, ReadRole);
    }
}

Article::Status Article::status() const {
    return m_status;
}

void Article::setStatus(Article::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged();
        emit dataChanged(this, StatusRole);
    }
}

QString Article::subscriptionId() const {
    return m_subscriptionId;
}

void Article::setSubscriptionId(const QString &i) {
    if (i != subscriptionId()) {
        m_subscriptionId = i;
        emit subscriptionIdChanged();
        emit dataChanged(this, SubscriptionIdRole);
    }
}

QString Article::title() const {
    return m_title;
}

void Article::setTitle(const QString &t) {
    if (t != title()) {
        m_title = t;
        emit titleChanged();
        emit dataChanged(this, TitleRole);
    }
}

QString Article::url() const {
    return m_url;
}

void Article::setUrl(const QString &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged();
        emit dataChanged(this, UrlRole);
    }
}

bool Article::autoUpdate() const {
    return m_autoUpdate;
}

void Article::setAutoUpdate(bool enabled) {
    if (enabled != autoUpdate()) {
        m_autoUpdate = enabled;
        emit autoUpdateChanged();
        emit dataChanged(this, AutoUpdateRole);

        if (enabled) {
            connect(DBNotify::instance(), SIGNAL(articleFavourited(QString, bool)),
                    this, SLOT(onArticleFavourited(QString, bool)));
            connect(DBNotify::instance(), SIGNAL(articleRead(QString, QString, bool)),
                    this, SLOT(onArticleRead(QString, QString, bool)));
            connect(DBNotify::instance(), SIGNAL(subscriptionRead(QString, bool)),
                    this, SLOT(onSubscriptionRead(QString, bool)));
            connect(DBNotify::instance(), SIGNAL(allSubscriptionsRead()), this, SLOT(onAllSubscriptionsRead()));
        }
        else {
            disconnect(DBNotify::instance(), 0, this, 0);
        }
    }
}

void Article::load(const QString &id) {
    setId(id);
    setStatus(Active);
    DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->fetchArticle(id);
}

void Article::markFavourite(bool favourite) {
    if (favourite != isFavourite()) {
        DBConnection *connection = DBConnection::connection();
        connect(connection, SIGNAL(finished(DBConnection*)), connection, SLOT(deleteLater()));
        connection->markArticleFavourite(id(), favourite);
    }
}

void Article::markRead(bool read) {
    if (read != isRead()) {
        DBConnection *connection = DBConnection::connection();
        connect(connection, SIGNAL(finished(DBConnection*)), connection, SLOT(deleteLater()));
        connection->markArticleRead(id(), read);
    }
}

void Article::remove() {
    DBConnection *connection = DBConnection::connection();
    connect(connection, SIGNAL(finished(DBConnection*)), connection, SLOT(deleteLater()));
    connection->deleteArticle(id());
}

void Article::onArticleFetched(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {        
        setAuthor(connection->value(1).toString());
        setBody(connection->value(2).toString());
        setCategories(connection->value(3).toString().split(", ", QString::SkipEmptyParts));
        setDate(QDateTime::fromTime_t(connection->value(4).toInt()));
        setEnclosures(QtJson::Json::parse(connection->value(5).toString()).toList());
        setFavourite(connection->value(6).toBool());
        setRead(connection->value(7).toBool());
        setSubscriptionId(connection->value(8).toString());
        setTitle(connection->value(9).toString());
        setUrl(connection->value(10).toString());
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

void Article::onArticleFavourited(const QString &articleId, bool isFavourite) {
    if (articleId == id()) {
        setFavourite(isFavourite);
    }
}

void Article::onArticleRead(const QString &articleId, const QString &, bool isRead) {
    if (articleId == id()) {
        setRead(isRead);
    }
}

void Article::onSubscriptionRead(const QString &subscriptionId, bool isRead) {
    if (subscriptionId == this->subscriptionId()) {
        setRead(isRead);
    }
}

void Article::onAllSubscriptionsRead() {
    setRead(true);
}
