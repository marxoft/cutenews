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
#include <QVariantMap>
#include <QNetworkRequest>

class QNetworkAccessManager;
class QNetworkReply;

class DBConnection : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QVariant result READ result NOTIFY finished)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)

public:
    enum Status {
        Idle = 0,
        Active,
        Canceled,
        Ready,
        Error
    };

    explicit DBConnection(QObject *parent = 0);
    ~DBConnection();
    
    QString errorString() const;
    
    int progress() const;
    
    QVariant result() const;
    
    Status status() const;
    
    static DBConnection* connection();
    static DBConnection* connection(QObject *obj, const char *slot);

public Q_SLOTS:
    void addSubscription(const QVariantMap &properties);
    void addSubscriptions(const QVariantList &subscriptions);
    void deleteSubscription(const QString &id);
    void updateSubscription(const QString &id, const QVariantMap &properties);
    void markSubscriptionRead(const QString &id, bool isRead = true);
    void markAllSubscriptionsRead();
    
    void fetchSubscription(const QString &id);
    void fetchSubscriptions(int offset = 0, int limit = 0);
    void fetchSubscriptions(const QStringList &ids);
    void fetchSubscriptions(const QVariantMap &params);
    
    void deleteArticle(const QString &id);
    void deleteReadArticles(int expiryDate);
    void markArticleFavourite(const QString &id, bool isFavourite = true);
    void markArticleRead(const QString &id, bool isRead = true);
    
    void fetchArticle(const QString &id);
    void fetchArticles(int offset = 0, int limit = 0);
    void fetchArticles(const QStringList &ids);
    void fetchArticles(const QVariantMap &params);
    void fetchArticlesForSubscription(const QString &subscriptionId, int offset = 0, int limit = 0);
    void searchArticles(const QString &query, int offset = 0, int limit = 0);
    
    void close();    

private Q_SLOTS:
    void onSubscriptionsAdded(QNetworkReply *reply);
    void onSubscriptionDeleted(QNetworkReply *reply);
    void onSubscriptionUpdated(QNetworkReply *reply);
    void onSubscriptionRead(QNetworkReply *reply);
    void onAllSubscriptionsRead(QNetworkReply *reply);
    
    void onArticleDeleted(QNetworkReply *reply);
    void onArticleFavourited(QNetworkReply *reply);
    void onArticleRead(QNetworkReply *reply);
    void onReadArticlesDeleted(QNetworkReply *reply);
    
    void onReplyProgress(qint64 current, qint64 total);
    void onReplyFinished(QNetworkReply *reply);

Q_SIGNALS:
    void finished(DBConnection *conn);
    void progressChanged(int p);
    void statusChanged(DBConnection::Status s);

private:    
    void setErrorString(const QString &e);
    
    void setProgress(int p);
    
    void setResult(const QVariant &r);
    
    void setStatus(Status s);
        
    QNetworkAccessManager* networkAccessManager(const char *slot);
    
    QNetworkAccessManager *m_nam;
    
    QString m_errorString;
    
    int m_progress;
    
    QVariant m_result;
    
    Status m_status;
};

#endif // DBCONNECTION_H
