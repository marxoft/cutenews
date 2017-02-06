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

#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <QObject>
#include <QSqlQuery>
#include <QVariantMap>

class QSqlDatabase;
class QThread;

class DBConnection : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool asynchronous READ isAsynchronous CONSTANT)
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)

public:
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Error
    };

    explicit DBConnection(bool asynchronous = false);
    ~DBConnection();

    bool isAsynchronous() const;
    
    QString errorString() const;
    
    int progress() const;
    
    Status status() const;
    
    Q_INVOKABLE static DBConnection* connection();
    static DBConnection* connection(QObject *obj, const char *slot);
    
    static QThread* asynchronousThread();
    static void setAsynchronousThread(QThread *thread);

    static const QString SUBSCRIPTION_FIELDS;
    static const QString ARTICLE_FIELDS;

public Q_SLOTS:
    void addSubscription(const QVariantList &properties);
    void addSubscriptions(const QList<QVariantList> &subscriptions);
    void deleteSubscription(const QString &id);
    void updateSubscription(const QString &id, const QVariantMap &properties, bool fetchResult = false);
    void markSubscriptionRead(const QString &id, bool isRead = true, bool fetchResult = false);
    void markAllSubscriptionsRead();
    
    void fetchSubscription(const QString &id);
    void fetchSubscriptions(int offset = 0, int limit = 0);
    void fetchSubscriptions(const QStringList &ids);
    void fetchSubscriptions(const QString &criteria);
    
    void addArticle(const QVariantList &properties, const QString &subscriptionId);
    void addArticles(const QList<QVariantList> &articles, const QString &subscriptionId);
    void deleteArticle(const QString &id);
    void deleteReadArticles(int expiryDate);
    void updateArticle(const QString &id, const QVariantMap &properties, bool fetchResult = false);
    void markArticleFavourite(const QString &id, bool isFavourite = true, bool fetchResult = false);
    void markArticleRead(const QString &id, bool isRead = true, bool fetchResult = false);
    
    void fetchArticle(const QString &id);
    void fetchArticles(int offset = 0, int limit = 0);
    void fetchArticles(const QStringList &ids);
    void fetchArticles(const QString &criteria);
    void fetchArticlesForSubscription(const QString &subscriptionId, int offset = 0, int limit = 0);
    void fetchFavouriteArticles(int offset = 0, int limit = 0);
    void fetchUnreadArticles(int offset = 0, int limit = 0);
    void searchArticles(const QString &query, int offset = 0, int limit = 0);
    
    void exec(const QString &statement);

    void clear();
    void close();
    
    bool nextRecord();
    
    int numRowsAffected() const;
    int size() const;
    
    QVariant value(int index) const;
    QVariant value(const QString &name) const;

private Q_SLOTS:
    void _p_addSubscription(const QVariantList &properties);
    void _p_addSubscriptions(const QList<QVariantList> &subscriptions);
    void _p_deleteSubscription(const QString &id);
    void _p_updateSubscription(const QString &id, const QVariantMap &properties, bool fetchResult);
    void _p_markSubscriptionRead(const QString &id, bool isRead, bool fetchResult);
    void _p_markAllSubscriptionsRead();
    
    void _p_fetchSubscription(const QString &id);
    void _p_fetchSubscriptions(int offset, int limit);
    void _p_fetchSubscriptions(const QStringList &ids);
    void _p_fetchSubscriptions(const QString &criteria);
    
    void _p_addArticle(const QVariantList &properties, const QString &subscriptionId);
    void _p_addArticles(const QList<QVariantList> &articles, const QString &subscriptionId);
    void _p_deleteArticle(const QString &id);
    void _p_deleteReadArticles(int expiryDate);
    void _p_updateArticle(const QString &id, const QVariantMap &properties, bool fetchResult);
    void _p_markArticleFavourite(const QString &id, bool isFavourite, bool fetchResult);
    void _p_markArticleRead(const QString &id, bool isRead, bool fetchResult);
    
    void _p_fetchArticle(const QString &id);
    void _p_fetchArticles(int offset, int limit);
    void _p_fetchArticles(const QStringList &ids);
    void _p_fetchArticles(const QString &criteria);
    void _p_fetchArticlesForSubscription(const QString &subscriptionId, int offset, int limit);
    void _p_fetchFavouriteArticles(int offset, int limit);
    void _p_fetchUnreadArticles(int offset, int limit);
    void _p_searchArticles(const QString &query, int offset, int limit);
    
    void _p_exec(const QString &statement);

Q_SIGNALS:
    void finished(DBConnection *conn);
    void progressChanged(int p);
    void statusChanged(DBConnection::Status s);

private:
    void setErrorString(const QString &e);
    
    void setStatus(Status s);
    
    void setProgress(int p);
    
    QSqlDatabase database();
    
    static QThread *asyncThread;

    bool m_asynchronous;
    
    QString m_errorString;
    
    int m_progress;
    
    Status m_status;
    
    QSqlQuery m_query;
};

#endif // DBCONNECTION_H
