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

#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <QObject>
#include <QDateTime>
#include <QUrl>
#include <QVariant>

class QSqlQuery;

class Subscription : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int id READ id NOTIFY idChanged)
    Q_PROPERTY(int cacheSize READ cacheSize NOTIFY cacheSizeChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(bool downloadEnclosures READ downloadEnclosures NOTIFY downloadEnclosuresChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(QString iconPath READ iconPath NOTIFY iconPathChanged)
    Q_PROPERTY(QDateTime lastUpdated READ lastUpdated NOTIFY lastUpdatedChanged)
    Q_PROPERTY(bool read READ isRead NOTIFY unreadArticlesChanged)
    Q_PROPERTY(QVariant source READ source NOTIFY sourceChanged)
    Q_PROPERTY(SourceType sourceType READ sourceType NOTIFY sourceTypeChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(int updateInterval READ updateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)
    Q_PROPERTY(int unreadArticles READ unreadArticles NOTIFY unreadArticlesChanged)
    
    Q_ENUMS(SourceType Status)

public:
    enum SourceType {
        Url = 0,
        LocalFile,
        Command,
        Plugin,
        None
    };
    
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Error
    };
    
    explicit Subscription(QObject *parent = 0);
    explicit Subscription(const QSqlQuery &query, QObject *parent = 0);
    explicit Subscription(int id, int cacheSize, const QString &description, bool downloadEnclosures,
                          const QString &iconPath, const QDateTime &lastUpdated, const QVariant &source,
                          SourceType sourceType, const QString &title, int updateInterval, const QUrl &url,
                          int unreadArticles, QObject *parent = 0);
    
    int id() const;
    
    int cacheSize() const;
    
    QString description() const;
    
    bool downloadEnclosures() const;
    
    QString errorString() const;
        
    QString iconPath() const;
    
    QDateTime lastUpdated() const;
    
    bool isRead() const;
    
    QVariant source() const;
    SourceType sourceType() const;
    
    Status status() const;
    
    QString title() const;
        
    int updateInterval() const;
    
    QUrl url() const;
    
    int unreadArticles() const;

public Q_SLOTS:
    void load(int id);

private Q_SLOTS:
    void onArticlesAdded(int count, int subscriptionId);
    void onArticleDeleted(int articleId, int subscriptionId);
    void onArticleRead(int articleId, int subscriptionId, bool isRead);
    void onSubscriptionFetched(const QSqlQuery &query, int requestId);
    void onSubscriptionRead(int subscriptionId, bool isRead);
    void onSubscriptionUpdated(int subscriptionId);

private:
    void setId(int i);
    
    void setCacheSize(int s);
    
    void setDescription(const QString &d);
    
    void setDownloadEnclosures(bool d);
    
    void setErrorString(const QString &e);
    
    void setIconPath(const QString &p);
    
    void setLastUpdated(const QDateTime &d);
    
    void setSource(const QVariant &s);
    void setSourceType(SourceType t);
    
    void setStatus(Status s);
    
    void setTitle(const QString &t);
        
    void setUpdateInterval(int i);
    
    void setUrl(const QUrl &u);
    
    void setUnreadArticles(int u);

Q_SIGNALS:
    void idChanged();
    void cacheSizeChanged();
    void descriptionChanged();
    void dataChanged(Subscription *subscription);
    void downloadEnclosuresChanged();
    void iconPathChanged();
    void lastUpdatedChanged();
    void readChanged();
    void sourceChanged();
    void sourceTypeChanged();
    void statusChanged();
    void titleChanged();
    void updateIntervalChanged();
    void urlChanged();
    void unreadArticlesChanged();

private:
    int m_id;
    
    int m_cacheSize;
       
    QString m_description;
    
    bool m_downloadEnclosures;
    
    QString m_errorString;
        
    QString m_iconPath;
    
    QDateTime m_lastUpdated;
        
    QVariant m_source;
    
    SourceType m_sourceType;
    
    Status m_status;

    QString m_title;
        
    int m_updateInterval;
    
    QUrl m_url;
    
    int m_unreadArticles;
};

#endif // SUBSCRIPTION_H
