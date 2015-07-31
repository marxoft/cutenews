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

#include "articlemodel.h"
#include "article.h"
#include "database.h"
#include "definitions.h"
#include <QSqlError>

ArticleModel::ArticleModel(QObject *parent) :
    QAbstractListModel(parent),
    m_status(Idle),
    m_subscriptionId(ALL_ARTICLES_SUBSCRIPTION_ID)
{
    m_roles[IdRole] = "id";
    m_roles[AuthorRole] = "author";
    m_roles[BodyRole] = "body";
    m_roles[CategoriesRole] = "categories";
    m_roles[DateRole] = "date";
    m_roles[EnclosuresRole] = "enclosures";
    m_roles[FavouriteRole] = "favourite";
    m_roles[ReadRole] = "read";
    m_roles[SubscriptionIdRole] = "subscriptionId";
    m_roles[TitleRole] = "title";
    m_roles[UrlRole] = "url";
#if QT_VERSION <= 0x050000
    setRoleNames(m_roles);
#endif
    connect(Database::instance(), SIGNAL(articlesAdded(int, int)), this, SLOT(onArticlesAdded(int, int)));
    connect(Database::instance(), SIGNAL(articleDeleted(int, int)), this, SLOT(onArticleDeleted(int, int)));
    connect(Database::instance(), SIGNAL(articleFavourited(int, bool)), this, SLOT(onArticleFavourited(int, bool)));
    connect(Database::instance(), SIGNAL(articlesFetched(QSqlQuery, int)),
            this, SLOT(onArticlesFetched(QSqlQuery, int)));
    connect(Database::instance(), SIGNAL(subscriptionDeleted(int)), this, SLOT(onSubscriptionDeleted(int)));
}

QString ArticleModel::errorString() const {
    return m_errorString;
}

void ArticleModel::setErrorString(const QString &e) {
    m_errorString = e;
}

ArticleModel::Status ArticleModel::status() const {
    return m_status;
}

void ArticleModel::setStatus(ArticleModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> ArticleModel::roleNames() const {
    return m_roles;
}
#endif

int ArticleModel::rowCount(const QModelIndex &) const {
    return m_list.size();
}

QVariant ArticleModel::data(const QModelIndex &index, int role) const {
    if (Article *article = get(index.row())) {
        return article->property(m_roles.value(role));
    }
    
    return QVariant();
}

QVariant ArticleModel::data(int row, const QByteArray &role) const {
    if (Article *article = get(row)) {
        return article->property(role);
    }
    
    return QVariant();
}

QMap<int, QVariant> ArticleModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
        
    if (Article *article = get(index.row())) {
        QHashIterator<int, QByteArray> iterator(m_roles);
        
        while (iterator.hasNext()) {
            iterator.next();
            map[iterator.key()] = article->property(iterator.value());
        }
    }
    
    return map;
}

QVariantMap ArticleModel::itemData(int row) const {
    QVariantMap map;
        
    if (Article *article = get(row)) {
        foreach (QByteArray role, m_roles.values()) {
            map[role] = article->property(role);
        }
    }
    
    return map;
}

Article* ArticleModel::get(int row) const {
    if ((row >= 0) && (row < m_list.size())) {
        return m_list.at(row);
    }
    
    return 0;
}

int ArticleModel::match(const QByteArray &role, const QVariant &value) const {
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list.at(i)->property(role) == value) {
            return i;
        }
    }
    
    return -1;
}

void ArticleModel::clear() {
    if (!m_list.isEmpty()) {
        beginResetModel();
        qDeleteAll(m_list);
        m_list.clear();
        endResetModel();
        emit countChanged(0);
        setStatus(Idle);
    }
}

void ArticleModel::load(int subscriptionId) {
    if (status() == Active) {
        return;
    }
    
    m_subscriptionId = subscriptionId;
    m_query = QString();
    clear();
    setStatus(Active);
    
    switch (subscriptionId) {
    case ALL_ARTICLES_SUBSCRIPTION_ID:
        Database::fetchArticles(QString("ORDER BY date"), m_subscriptionId);
        break;
    case FAVOURITES_SUBSCRIPTION_ID:
        Database::fetchArticles(QString("WHERE isFavourite = 1 ORDER BY date"), m_subscriptionId);
        break;
    default:
        Database::fetchArticles(QString("WHERE subscriptionId = %1 ORDER BY date").arg(m_subscriptionId),
                                m_subscriptionId);
        break;
    }
}

void ArticleModel::search(const QString &query) {
    if (status() == Active) {
        return;
    }
    
    m_query = query;
    m_subscriptionId = -1000;
    clear();
    setStatus(Active);
    Database::fetchArticles(query, m_subscriptionId);
}

void ArticleModel::reload() {
    if (!m_query.isEmpty()) {
        search(m_query);
    }
    else {
        load(m_subscriptionId);
    }
}

void ArticleModel::onArticleChanged(Article *article) {
    const int i = m_list.indexOf(article);
    
    if (i >= 0) {
        const QModelIndex idx = index(i);
        emit dataChanged(idx, idx);
    }
}

void ArticleModel::onArticlesAdded(int count, int subscriptionId) {
    if ((status() == Ready) && ((subscriptionId == m_subscriptionId)
                                || (m_subscriptionId == ALL_ARTICLES_SUBSCRIPTION_ID))) {
        setStatus(Active);
        Database::fetchArticles(QString("WHERE subscriptionId = %1 ORDER BY id DESC LIMIT %2")
                                .arg(subscriptionId).arg(count), m_subscriptionId);
    }
}

void ArticleModel::onArticleDeleted(int articleId, int subscriptionId) {
    if ((subscriptionId == m_subscriptionId) || (m_subscriptionId == ALL_ARTICLES_SUBSCRIPTION_ID)
        || (m_subscriptionId == FAVOURITES_SUBSCRIPTION_ID)) {
        for (int i = 0; i < m_list.size(); i++) {
            if (m_list.at(i)->id() == articleId) {
                beginRemoveRows(QModelIndex(), i, i);
                m_list.takeAt(i)->deleteLater();
                endRemoveRows();
                emit countChanged(rowCount());
                return;
            }
        }
    }
}

void ArticleModel::onArticleFavourited(int articleId, bool isFavourite) {
    if ((m_subscriptionId == FAVOURITES_SUBSCRIPTION_ID) && (status() == Ready)) {
        if (isFavourite) {
            setStatus(Active);
            Database::fetchArticles(QString("WHERE id = %1").arg(articleId), m_subscriptionId);
        }
        else {
            for (int i = 0; i < m_list.size(); i++) {
                if (m_list.at(i)->id() == articleId) {
                    beginRemoveRows(QModelIndex(), i, i);
                    m_list.takeAt(i)->deleteLater();
                    endRemoveRows();
                    emit countChanged(rowCount());
                    return;
                }
            }
        }
    }
}

void ArticleModel::onArticlesFetched(QSqlQuery query, int requestId) {
    if (requestId == m_subscriptionId) {
        if (query.lastError().isValid()) {
            setErrorString(query.lastError().text());
            setStatus(Error);
            return;
        }
        
        while (query.next()) {
            beginInsertRows(QModelIndex(), 0, 0);
            Article *article = new Article(query, this);
            connect(article, SIGNAL(dataChanged(Article*)), this, SLOT(onArticleChanged(Article*)));
            m_list.prepend(article);
            endInsertRows();
        }
        
        emit countChanged(rowCount());
        setStatus(Ready);
    }
}

void ArticleModel::onSubscriptionDeleted(int id) {
    if (id == m_subscriptionId) {
        clear();
    }
}
