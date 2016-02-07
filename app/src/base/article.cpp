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
#include "database.h"
#include <QSqlQuery>
#include <QSqlError>

Article::Article(QObject *parent) :
    QObject(parent),
    m_id(-1),
    m_favourite(false),
    m_read(false),
    m_status(Idle),
    m_subscriptionId(-1)
{
    connect(Database::instance(), SIGNAL(articleFavourited(int, bool)), this, SLOT(onArticleFavourited(int, bool)));
    connect(Database::instance(), SIGNAL(articleRead(int, int, bool)), this, SLOT(onArticleRead(int, int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionRead(int, bool)), this, SLOT(onSubscriptionRead(int, bool)));
}

Article::Article(const QSqlQuery &query, QObject *parent) :
    QObject(parent),
    m_id(Database::articleId(query)),
    m_author(Database::articleAuthor(query)),
    m_body(Database::articleBody(query)),
    m_categories(Database::articleCategories(query)),
    m_date(Database::articleDate(query)),
    m_enclosures(Database::articleEnclosures(query)),
    m_favourite(Database::articleIsFavourite(query)),
    m_read(Database::articleIsRead(query)),
    m_status(Ready),
    m_subscriptionId(Database::articleSubscriptionId(query)),
    m_title(Database::articleTitle(query)),
    m_url(Database::articleUrl(query))
{
    connect(Database::instance(), SIGNAL(articleFavourited(int, bool)), this, SLOT(onArticleFavourited(int, bool)));
    connect(Database::instance(), SIGNAL(articleRead(int, int, bool)), this, SLOT(onArticleRead(int, int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionRead(int, bool)), this, SLOT(onSubscriptionRead(int, bool)));
}

Article::Article(int id, const QString &author, const QString &body, const QStringList &categories,
                 const QDateTime &date, const QVariantList &enclosures, bool isFavourite, bool isRead,
                 int subscriptionId, const QString &title, const QUrl &url, QObject *parent) :
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
    m_url(url)
{
    connect(Database::instance(), SIGNAL(articleFavourited(int, bool)), this, SLOT(onArticleFavourited(int, bool)));
    connect(Database::instance(), SIGNAL(articleRead(int, int, bool)), this, SLOT(onArticleRead(int, int, bool)));
    connect(Database::instance(), SIGNAL(subscriptionRead(int, bool)), this, SLOT(onSubscriptionRead(int, bool)));
}

int Article::id() const {
    return m_id;
}

void Article::setId(int i) {
    if (i != id()) {
        m_id = i;
        emit idChanged();
        emit dataChanged(this);
    }
}

QString Article::author() const {
    return m_author;
}

void Article::setAuthor(const QString &a) {
    if (a != author()) {
        emit authorChanged();
        emit dataChanged(this);
    }
}

QString Article::body() const {
    return m_body;
}

void Article::setBody(const QString &b) {
    if (b != body()) {
        m_body = b;
        emit bodyChanged();
        emit dataChanged(this);
    }
}

QStringList Article::categories() const {
    return m_categories;
}

void Article::setCategories(const QStringList &c) {
    m_categories = c;
    emit categoriesChanged();
    emit dataChanged(this);
}

QDateTime Article::date() const {
    return m_date;
}

void Article::setDate(const QDateTime &d) {
    if (d != date()) {
        m_date = d;
        emit dateChanged();
        emit dataChanged(this);
    }
}

QVariantList Article::enclosures() const {
    return m_enclosures;
}

void Article::setEnclosures(const QVariantList &e) {
    m_enclosures = e;
    emit enclosuresChanged();
    emit dataChanged(this);
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
        emit dataChanged(this);
    }
}

bool Article::isRead() const {
    return m_read;
}

void Article::setRead(bool r) {
    if (r != isRead()) {
        m_read = r;
        emit readChanged();
        emit dataChanged(this);
    }
}

Article::Status Article::status() const {
    return m_status;
}

void Article::setStatus(Article::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged();
    }
}

int Article::subscriptionId() const {
    return m_subscriptionId;
}

void Article::setSubscriptionId(int i) {
    if (i != subscriptionId()) {
        m_subscriptionId = i;
        emit subscriptionIdChanged();
        emit dataChanged(this);
    }
}

QString Article::title() const {
    return m_title;
}

void Article::setTitle(const QString &t) {
    if (t != title()) {
        m_title = t;
        emit titleChanged();
        emit dataChanged(this);
    }
}

QUrl Article::url() const {
    return m_url;
}

void Article::setUrl(const QUrl &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged();
        emit dataChanged(this);
    }
}

void Article::load(int id) {
    setId(id);
    setStatus(Active);
    connect(Database::instance(), SIGNAL(articleFetched(QSqlQuery, int)), this, SLOT(onArticleFetched(QSqlQuery, int)));
    Database::fetchArticle(id, id);
}

void Article::onArticleFetched(const QSqlQuery &query, int requestId) {
    if (requestId == id()) {
        disconnect(Database::instance(), SIGNAL(articleFetched(QSqlQuery, int)),
                   this, SLOT(onArticleFetched(QSqlQuery, int)));
        
        setAuthor(Database::articleAuthor(query));
        setBody(Database::articleBody(query));
        setCategories(Database::articleCategories(query));
        setDate(Database::articleDate(query));
        setEnclosures(Database::articleEnclosures(query));
        setFavourite(Database::articleIsFavourite(query));
        setRead(Database::articleIsRead(query));
        setSubscriptionId(Database::articleSubscriptionId(query));
        setTitle(Database::articleTitle(query));
        setUrl(Database::articleUrl(query));
        setErrorString(query.lastError().text());
        setStatus(query.lastError().isValid() ? Error : Ready);
    }
}

void Article::onArticleFavourited(int articleId, bool isFavourite) {
    if (articleId == id()) {
        setFavourite(isFavourite);
    }
}

void Article::onArticleRead(int articleId, int, bool isRead) {
    if (articleId == id()) {
        setRead(isRead);
    }
}

void Article::onSubscriptionRead(int subscriptionId, bool isRead) {
    if (subscriptionId == this->subscriptionId()) {
        setRead(isRead);
    }
}
