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

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QVariantMap>

class QDateTime;
class QSqlQuery;
class QUrl;

class Database : public QObject
{
    Q_OBJECT
    
public:
    ~Database();
    
    static Database* instance();
    
    static int subscriptionId(const QSqlQuery &query);
    static int subscriptionCacheSize(const QSqlQuery &query);
    static QString subscriptionDescription(const QSqlQuery &query);
    static bool subscriptionDownloadEnclosures(const QSqlQuery &query);
    static QString subscriptionIconPath(const QSqlQuery &query);
    static QDateTime subscriptionLastUpdated(const QSqlQuery &query);
    static QString subscriptionSource(const QSqlQuery &query);
    static int subscriptionSourceType(const QSqlQuery &query);
    static QString subscriptionTitle(const QSqlQuery &query);
    static int subscriptionUpdateInterval(const QSqlQuery &query);
    static QUrl subscriptionUrl(const QSqlQuery &query);
    static int subscriptionUnreadArticles(const QSqlQuery &query);
    
    static int articleId(const QSqlQuery &query);
    static QString articleAuthor(const QSqlQuery &query);
    static QString articleBody(const QSqlQuery &query);
    static QStringList articleCategories(const QSqlQuery &query);
    static QDateTime articleDate(const QSqlQuery &query);
    static QVariantList articleEnclosures(const QSqlQuery &query);
    static bool articleIsFavourite(const QSqlQuery &query);
    static bool articleIsRead(const QSqlQuery &query);
    static int articleSubscriptionId(const QSqlQuery &query);
    static QString articleTitle(const QSqlQuery &query);
    static QUrl articleUrl(const QSqlQuery &query);
        
    Q_INVOKABLE static QString errorString();

public Q_SLOTS:
    static bool init();
    
    static bool addSubscription(const QVariantList &properties);
    static bool deleteSubscription(int id);
    static bool updateSubscription(int id, const QVariantMap &properties);
    static bool markSubscriptionRead(int id, bool isRead);
    
    static bool fetchSubscription(int subscriptionId, int requestId);
    static bool fetchSubscriptions(int requestId);
    static bool fetchSubscriptions(const QString &criteria, int requestId);
    
    static bool addArticle(const QVariantList &properties, int subscriptionId);
    static bool addArticles(const QList<QVariantList> &articles, int subscriptionId);
    static bool deleteArticle(int id);
    static bool markArticleFavourite(int id, bool isFavourite);
    static bool markArticleRead(int id, bool isRead);
    
    static bool fetchArticle(int articleId, int requestId);
    static bool fetchArticles(int requestId);
    static bool fetchArticles(const QString &criteria, int requestId);

private Q_SLOTS:
    void _p_addSubscription(const QVariantList &properties);
    void _p_deleteSubscription(int id);
    void _p_updateSubscription(int id, const QVariantMap &properties);
    void _p_markSubscriptionRead(int id, bool isRead);
    
    void _p_fetchSubscription(int subscriptionId, int requestId);
    void _p_fetchSubscriptions(int requestId);
    void _p_fetchSubscriptions(const QString &criteria, int requestId);
    
    void _p_addArticle(const QVariantList &properties, int subscriptionId);
    void _p_addArticles(const QList<QVariantList> &articles, int subscriptionId);
    void _p_deleteArticle(int id);
    void _p_markArticleFavourite(int id, bool isFavourite);
    void _p_markArticleRead(int id, bool isRead);
    
    void _p_fetchArticle(int articleId, int requestId);
    void _p_fetchArticles(int requestId);
    void _p_fetchArticles(const QString &criteria, int requestId);
    
Q_SIGNALS:
    void subscriptionAdded(int id);
    void subscriptionDeleted(int id);
    void subscriptionUpdated(int id);
    void subscriptionRead(int id, bool isRead);
    
    void subscriptionFetched(const QSqlQuery &query, int requestId);
    void subscriptionsFetched(const QSqlQuery &query, int requestId);
    
    void articlesAdded(int count, int subscriptionId);
    void articleDeleted(int articleId, int subscriptionId);
    void articleFavourited(int id, bool isFavourite);
    void articleRead(int articleId, int subscriptionId, bool isRead);
    
    void articleFetched(const QSqlQuery &query, int requestId);
    void articlesFetched(const QSqlQuery &query, int requestId);
    
    void error(const QString &errorString);

private:
    Database();
    
    static Database *self;
};

#endif // DATABASE_H
